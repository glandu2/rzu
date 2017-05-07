#include "ClassCounter.h"

#include "AuthServerSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AuthServerSession)

#include "ClientSession.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ClientSession)

#include "StateHandler/GameHandler/GameHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::GameHandler)

#include "StateHandler/LobbyHandler/LobbyHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::LobbyHandler)

#include "Component/Character/Character.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::Character)

#include "StateHandler/PlayerLoadingHandler/PlayerLoadingHandler.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::PlayerLoadingHandler)

#include "ReferenceData/ReferenceDataMgr.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ReferenceDataMgr)

#include "ReferenceData/BannedWords.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::BannedWordsBinding)

#include "ReferenceData/ItemResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::ItemResourceBinding)

#include "ReferenceData/JobLevelBonus.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::JobLevelBonusBinding)

#include "ReferenceData/JobResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::JobResourceBinding)

#include "ReferenceData/StatResource.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::StatResourceBinding)

#include "ReferenceData/ObjectSidState.h"
DECLARE_CLASSCOUNT_STATIC(GameServer::AwakenOptionSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::FarmSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::ItemSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::PetSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::SkillSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::SummonSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::TitleSidBinding)
DECLARE_CLASSCOUNT_STATIC(GameServer::TitleConditionSidBinding)
