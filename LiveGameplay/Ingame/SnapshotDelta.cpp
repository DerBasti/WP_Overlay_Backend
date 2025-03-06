#include "SnapshotDelta.h"
#include "../../Common/Data/ItemDatabase.h"
#include "../../Common/PerformanceClock.h"
#include <math.h>
#include <future>

SnapshotDelta::SnapshotDelta(RiotPlayerList* previous, RiotPlayerList* current, RiotEventList* previousEvents, RiotEventList* events) {
	teamKills.insert(std::make_pair(TeamType::BLUE, 0));
	teamKills.insert(std::make_pair(TeamType::RED, 0));

	lastEpicKillTimes.insert_or_assign(EpicMonsterType::BARON_NASHOR, 840.0f);
	lastEpicKillTimes.insert_or_assign(EpicMonsterType::RIFT_HERALD, 120.0f);
	lastEpicKillTimes.insert_or_assign(EpicMonsterType::DRAGON, 0.0f);
	lastEpicKillTimes.insert_or_assign(EpicMonsterType::ELDER_DRAGON, (std::numeric_limits<float>::max)());

#ifdef _DEBUG
	PerformanceClock timer;
#endif
	currentGameTime = 0.0f;
	std::future<uint64_t> handleTeams;
	if (previous != nullptr && current != nullptr && previous->getGametime() < current->getGametime()) {
		handleTeams = std::async(std::launch::async, [&]() {
			PerformanceClock timer;
			handleChampsFromTeam(previous->getBlueTeam(), current->getBlueTeam(), current->getGametime());
			handleChampsFromTeam(previous->getRedTeam(), current->getRedTeam(), current->getGametime());
			auto duration = timer.getDuration();
			return duration;
		});
	}
	if (current) {
		currentGameTime = current->getGametime();
	}
	auto inhibitorEventAsync = std::async(std::launch::async, [&]() {
		PerformanceClock clock;
		for (auto event : events->getEventsByName(RiotIngameEvent::INHIBITOR_KILLED_EVENT_NAME)) {
			InhibitorKilledEvent* realEvent = dynamic_cast<InhibitorKilledEvent*>(event.get());
			realEvent->updateRemainingTime(current->getGametime());
			if (realEvent->isActive()) {
				this->activelyRecoveringInhibs.push_back(realEvent);
			}
		}
		for (auto previousEvent : previousEvents->getEventsByName(RiotIngameEvent::INHIBITOR_KILLED_EVENT_NAME)) {
			bool foundInhibitor = false;
			InhibitorKilledEvent* previousInhibitorEvent = dynamic_cast<InhibitorKilledEvent*>(previousEvent.get());
			for (auto inhib : activelyRecoveringInhibs) {
				if (inhib->getInhibitor().getLanePosition() == previousInhibitorEvent->getInhibitor().getLanePosition() &&
					inhib->getInhibitor().getTeam() == previousInhibitorEvent->getInhibitor().getTeam()) {
					foundInhibitor = true;
					break;
				}
			}
			if (!foundInhibitor && previousInhibitorEvent->isActive()) {
				this->respawnedInhibitors.push_back(new InhibitorRespawnedEvent(RiotIngameEvent::INHIBITOR_RESPAWNED_EVENT_NAME, current->getGametime(), previousInhibitorEvent->getInhibitor()));
			}
		}
		return clock.getDuration();
	});
#ifdef _DEBUG
	auto inhibDuration = timer.getDuration();
	timer.reset();
#endif
	
	for (auto event : events->getEventsByName(RiotIngameEvent::BARON_KILLED_EVENT_NAME)) {
		if (event->getOccuranceTime() < currentGameTime) {
			lastEpicKillTimes.insert_or_assign(EpicMonsterType::BARON_NASHOR, event->getOccuranceTime());
		}
	}
#ifdef _DEBUG
	auto baronsKilledTimer = timer.getDuration();
	timer.reset();
#endif

	int drakesKillPerTeam[2] = { 0,0 };
	for (auto event : events->getEventsByName(RiotIngameEvent::DRAKE_KILLED_EVENT_NAME)) {
		if (event->getOccuranceTime() >= currentGameTime) {
			continue;
		}
		DragonKilledEvent* dragonEvent = dynamic_cast<DragonKilledEvent*>(event.get());
		dragonKillHistory.addDragonToTeam(dragonEvent->getBuffedTeam(), dragonEvent->getKilledDragonType(), dragonEvent->getOccuranceTime());
		if (dragonEvent->getKilledDragonType() == DragonType::ELDER || dragonKillHistory.isSoulpointAcquired()) {
			lastEpicKillTimes.insert_or_assign(EpicMonsterType::DRAGON, 0.0f);
			lastEpicKillTimes.insert_or_assign(EpicMonsterType::ELDER_DRAGON, dragonEvent->getOccuranceTime());
		}
		else {
			lastEpicKillTimes.insert_or_assign(EpicMonsterType::DRAGON, dragonEvent->getOccuranceTime());
		}
	}
#ifdef _DEBUG
	auto drakesKilledTimer = timer.getDuration();
	timer.reset();
#endif
	for (auto event : events->getEventsByName(RiotIngameEvent::RIFT_HERALD_KILLED_EVENT_NAME)) {
		if (event->getOccuranceTime() >= currentGameTime) {
			continue;
		}
		lastEpicKillTimes.insert_or_assign(EpicMonsterType::RIFT_HERALD, event->getOccuranceTime());
	}
	turretKills.insert(std::make_pair(TeamType::BLUE, events->getTurretsDestroyedByTeam(TeamType::BLUE)));
	turretKills.insert(std::make_pair(TeamType::RED, events->getTurretsDestroyedByTeam(TeamType::RED)));
	gameStarted = false;
	uint64_t inhibitorEventDuration = 0;
	if (inhibitorEventAsync.valid()) {
		inhibitorEventDuration = inhibitorEventAsync.get();
	}
	uint64_t handleTeamsDuration = 0;
	if (handleTeams.valid()) {
		handleTeamsDuration = handleTeams.get();
	}
}

SnapshotDelta::~SnapshotDelta() {
	for (auto levelup : levelups) {
		delete levelup;
	}
	levelups.clear();
	for (auto inhib : respawnedInhibitors) {
		delete inhib;
	}
	respawnedInhibitors.clear();
}

void SnapshotDelta::handleChampsFromTeam(Team* teamPreviousSnapshot, Team* teamCurrentSnapshot, float gametime) {
	for (auto itPair : teamPreviousSnapshot->getChampions()) {
		auto champPrevious = itPair.second;
		auto champCurrent = teamCurrentSnapshot->getChampionSnapshotById(champPrevious->getSpectatorSlotId());
		if (champPrevious->getLevel() < champCurrent->getLevel()) {
			this->levelups.insert(new LevelupSnapshotDelta(champCurrent, champPrevious->getLevel(), champCurrent->getLevel()));
		}
		std::vector<Item*> finishedItemsForChamp;
		for (uint8_t inventorySlot = 0; inventorySlot < 7; inventorySlot++) {
			auto currentItem = champCurrent->getInventory()[inventorySlot];
			if (currentItem != nullptr && (currentItem->isLegendary() || currentItem->isMythic()) && !champPrevious->doesInventoryItemExist(currentItem->getId())) {
				finishedItemsForChamp.push_back(currentItem);
			}
		}
		if (!finishedItemsForChamp.empty()) {
			this->itemsFinished.insert(std::make_pair(champCurrent, std::move(finishedItemsForChamp)));
		}
		auto currentKills = this->teamKills.at(teamPreviousSnapshot->getTeamType()) + champCurrent->getChampsKilled();
		this->teamKills[teamPreviousSnapshot->getTeamType()] = currentKills;
		this->creepsPerMinute.insert(std::make_pair(champCurrent, static_cast<float>(champCurrent->getCreepsKilled()) / (gametime/60.0f)));
	}
}