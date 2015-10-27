#include "ClassCounter.h"

#include "GameServer/AuthServerSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AuthServerSession)

#include "GameServer/ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ClientSession)

#include "GameServer/GameHandler/GameHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::GameHandler)

#include "GameServer/LobbyHandler/LobbyHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::LobbyHandler)

#include "GameServer/Model/Character.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::Character)

#include "GameServer/PlayerLoadingHandler/PlayerLoadingHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::PlayerLoadingHandler)

#include "GameServer/ReferenceData/ReferenceDataMgr.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ReferenceDataMgr)

#include "GameServer/ReferenceData/BannedWords.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::BannedWordsBinding)

#include "GameServer/ReferenceData/ItemResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ItemResourceBinding)

#include "GameServer/ReferenceData/JobLevelBonus.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::JobLevelBonusBinding)

#include "GameServer/ReferenceData/JobResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::JobResourceBinding)

#include "GameServer/ReferenceData/StatResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::StatResourceBinding)

#include "GameServer/ReferenceData/ObjectSidState.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AwakenOptionSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::FarmSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::ItemSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::PetSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::SkillSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::SummonSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::TitleSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::TitleConditionSidBinding)
