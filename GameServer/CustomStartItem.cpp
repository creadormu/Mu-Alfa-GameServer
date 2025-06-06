#include "stdafx.h"
#include "Util.h"
#include "User.h"
#include "CustomStartItem.h"
#include "CashShop.h"
#include "MemScript.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "SocketItemType.h"

CCustomStartItem gCustomStartItem;

CCustomStartItem::CCustomStartItem() // OK
{
	this->Init();
}

CCustomStartItem::~CCustomStartItem() // OK
{

}

void CCustomStartItem::Init() // OK
{
	for (int n = 0; n < MAX_CLASS; n++)
	{
		this->m_CustomStartItemInfo[n].Class = -1;
		this->m_CustomStartItemInfo[n].ItemStart.clear();
	}
}

void CCustomStartItem::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CUSTOM_START_ITEM_INFO Items;

					int Class = lpMemScript->GetNumber();

					Items.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(), lpMemScript->GetAsNumber()));

					Items.ItemLevel = lpMemScript->GetAsNumber();

					Items.ItemLuck = lpMemScript->GetAsNumber();

					Items.ItemDurability = lpMemScript->GetAsNumber();

					Items.ItemSkill = lpMemScript->GetAsNumber();

					Items.ItemOption = lpMemScript->GetAsNumber();

					Items.ItemExcellent = lpMemScript->GetAsNumber();

					Items.ItemAncient = lpMemScript->GetAsNumber();

					Items.ItemJOH = lpMemScript->GetAsNumber();

					Items.ItemOpEx = lpMemScript->GetAsNumber();

					Items.ItemSocket1 = lpMemScript->GetAsNumber();

					Items.ItemSocket2 = lpMemScript->GetAsNumber();

					Items.ItemSocket3 = lpMemScript->GetAsNumber();

					Items.ItemSocket4 = lpMemScript->GetAsNumber();

					Items.ItemSocket5 = lpMemScript->GetAsNumber();

					Items.ItemTime = lpMemScript->GetAsNumber();

					//this->SetInfo(info);

					this->m_CustomStartItemInfo[Class].ItemStart.push_back(Items);
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_START_BUFF_INFO buffs;

					int Class1 = lpMemScript->GetNumber();

					buffs.Type = lpMemScript->GetAsNumber();

					buffs.Effect = lpMemScript->GetAsNumber();

					buffs.Power1 = lpMemScript->GetAsNumber();

					buffs.Power2 = lpMemScript->GetAsNumber();

					buffs.Time = lpMemScript->GetAsNumber();

					this->m_CustomStartBuffInfo[Class1].BuffStart.push_back(buffs);
				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}


void CCustomStartItem::SetBuffs(CustomStartBuff_INFO buffs) // OK
{
	if (CHECK_RANGE(buffs.Class, MAX_CLASS) == 0)
	{
		return;
	}

	this->m_CustomStartBuffInfo[buffs.Class] = buffs;
}

bool CCustomStartItem::CheckSkillEffect(int m_index) // OK
{
	bool skill = false;
	switch(m_index)
	{
		case EFFECT_GREATER_DAMAGE:
			skill = true;
			break;
		case EFFECT_GREATER_DEFENSE:
			skill = true;
			break;
		case EFFECT_ELF_BUFFER:
			skill = true;
			break;
		case EFFECT_MANA_SHIELD:
			skill = true;
			break;
		case EFFECT_GREATER_CRITICAL_DAMAGE:
			skill = true;
			break;
		case EFFECT_INFINITY_ARROW:
			break;
		case EFFECT_BP_RECOVERY:
			break;
		case EFFECT_GREATER_LIFE:
			skill = true;
			break;
		case EFFECT_GREATER_MANA:
			skill = true;
			break;
		case EFFECT_BLESS_POTION:
			break;
		case EFFECT_SOUL_POTION:
			break;
		case EFFECT_DISABLE_MAGIC:
			break;
		case EFFECT_CASTLE_GATE_STATE:
			break;
		case EFFECT_GUILD_STATE1:
			break;
		case EFFECT_GUILD_STATE2:
			break;
		case EFFECT_GUILD_STATE3:
			break;
		case EFFECT_GUILD_STATE4:
			break;
		case EFFECT_INVISIBILITY:
			break;
		case EFFECT_GUILD_STATE5:
			break;
		case EFFECT_CASTLE_CROWN_STATE:
			break;
		case EFFECT_CRYWOLF_STATE1:
			break;
		case EFFECT_CRYWOLF_STATE2:
			break;
		case EFFECT_CRYWOLF_STATE3:
			break;
		case EFFECT_CRYWOLF_STATE4:
			break;
		case EFFECT_CRYWOLF_STATE5:
			break;
		case EFFECT_CRYWOLF_STATE6:
			break;
		case EFFECT_CRYWOLF_STATE7:
			break;
		case EFFECT_GAME_MASTER:
			break;
		case EFFECT_SEAL_OF_ASCENSION1:
			break;
		case EFFECT_SEAL_OF_WEALTH1:
			break;
		case EFFECT_SEAL_OF_SUSTENANCE1:
			break;
		case EFFECT_ORDER_OF_SPEED:
			break;
		case EFFECT_ORDER_OF_SUBLIMATION:
			break;
		case EFFECT_ORDER_OF_PROTECTION:
			break;
		case EFFECT_HALLOWEEN1:
			break;
		case EFFECT_HALLOWEEN2:
			break;
		case EFFECT_HALLOWEEN3:
			break;
		case EFFECT_HALLOWEEN4:
			break;
		case EFFECT_HALLOWEEN5:
			break;
		case EFFECT_SEAL_OF_ASCENSION2:
			break;
		case EFFECT_SEAL_OF_WEALTH2:
			break;
		case EFFECT_SEAL_OF_SUSTENANCE2:
			break;
		case EFFECT_SEAL_OF_MOVEMENT:
			break;
		case EFFECT_SCROLL_OF_QUICK:
			break;
		case EFFECT_SCROLL_OF_DEFENSE:
			break;
		case EFFECT_SCROLL_OF_DAMAGE:
			break;
		case EFFECT_SCROLL_OF_MAGIC_DAMAGE:
			break;
		case EFFECT_SCROLL_OF_LIFE:
			break;
		case EFFECT_SCROLL_OF_MANA:
			break;
		case EFFECT_ELIXIR_OF_STRENGTH:
			break;
		case EFFECT_ELIXIR_OF_DEXTERITY:
			break;
		case EFFECT_ELIXIR_OF_VITALITY:
			break;
		case EFFECT_ELIXIR_OF_ENERGY:
			break;
		case EFFECT_ELIXIR_OF_LEADERSHIP:
			break;
		case EFFECT_POISON:
			break;
		case EFFECT_ICE:
			break;
		case EFFECT_ICE_ARROW:
			break;
		case EFFECT_FIRE_SLASH:
			break;
		case EFFECT_PHYSI_DAMAGE_IMMUNITY:
			break;
		case EFFECT_MAGIC_DAMAGE_IMMUNITY:
			break;
		case EFFECT_STERN:
			break;
		case EFFECT_MAGIC_DEFENSE:
			break;
		case EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY:
			break;
		case EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY:
			break;
		case EFFECT_ORDER_OF_RESTRAINT:
			break;
		case EFFECT_CRYWOLF_STATE8:
			break;
		case EFFECT_CRYWOLF_STATE9:
			break;
		case EFFECT_CRYWOLF_STATE10:
			break;
		case EFFECT_CRYWOLF_STATE11:
			break;
		case EFFECT_CRYWOLF_STATE12:
			break;
		case EFFECT_DAMAGE_REFLECT:
			skill = true;
			break;
		case EFFECT_SLEEP:
			break;
		case EFFECT_BLIND:
			skill = true;
			break;
		case EFFECT_NEIL:
			break;
		case EFFECT_SAHAMUTT:
			break;
		case EFFECT_LESSER_DAMAGE:
			skill = true;
			break;
		case EFFECT_LESSER_DEFENSE:
			skill = true;
			break;
		case EFFECT_CHERRY_BLOSSOM1:
			break;
		case EFFECT_CHERRY_BLOSSOM2:
			break;
		case EFFECT_CHERRY_BLOSSOM3:
			break;
		case EFFECT_SWORD_POWER:
			skill = true;
			break;
		case EFFECT_MAGIC_CIRCLE:
			skill = true;
			break;
		case EFFECT_SWORD_SLASH:
			break;
			break;
		case EFFECT_RED_STORM:
			skill = true;
			break;
		case EFFECT_FROZEN_STAB:
			skill = true;
			break;
		case EFFECT_SEAL_OF_LIFE:
			break;
		case EFFECT_SEAL_OF_MANA:
			break;
		case EFFECT_SCROLL_OF_BATTLE:
			break;
		case EFFECT_SCROLL_OF_STRENGTH:
			break;
		case EFFECT_CHRISTMAS1:
			break;
		case EFFECT_CHRISTMAS2:
			break;
		case EFFECT_CHRISTMAS3:
			break;
		case EFFECT_CHRISTMAS4:
			break;
		case EFFECT_CHRISTMAS5:
			break;
		case EFFECT_CHRISTMAS6:
			break;
		case EFFECT_CHRISTMAS7:
			break;
		case EFFECT_DUEL_ARENA_WATCH:
			break;
		case EFFECT_TALISMAN_OF_GUARDIAN:
			break;
		case EFFECT_TALISMAN_OF_PROTECTION:
			break;
		case EFFECT_MASTER_SEAL_OF_ASCENSION:
			break;
		case EFFECT_MASTER_SEAL_OF_WEALTH:
			break;
		case EFFECT_GLADIATORS_GLORY:
			break;
		case EFFECT_PARTY_EXPERIENCE_BONUS:
			break;
		case EFFECT_MAX_AG_BOOST_AURA:
			break;
		case EFFECT_MAX_SD_BOOST_AURA:
			break;
		case EFFECT_SCROLL_OF_HEALING:
			break;
		case EFFECT_HAWK_FIGURINE:
			break;
		case EFFECT_GOAT_FIGURINE:
			break;
		case EFFECT_OAK_CHARM:
			break;
		case EFFECT_MAPLE_CHARM:
			break;
		case EFFECT_GOLDEN_OAK_CHARM:
			break;
		case EFFECT_GOLDEN_MAPLE_CHARM:
			break;
		case EFFECT_WORN_HORSESHOE:
			break;
		case EFFECT_GREATER_IGNORE_DEFENSE_RATE:
			skill = true;
			break;
		case EFFECT_FITNESS:
			skill = true;
			break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE:
			skill = true;
			break;
		case EFFECT_IRON_DEFENSE:
			skill = true;
			break;
		case EFFECT_GREATER_LIFE_ENHANCED:
			skill = true;
			break;
		case EFFECT_GREATER_LIFE_MASTERED:
			skill = true;
			break;
		case EFFECT_DEATH_STAB_ENHANCED:
			skill = true;
			break;
		case EFFECT_MAGIC_CIRCLE_IMPROVED:
			skill = true;
			break;
		case EFFECT_MAGIC_CIRCLE_ENHANCED:
			skill = true;
			break;
		case EFFECT_MANA_SHIELD_MASTERED:
			skill = true;
			break;
		case EFFECT_FROZEN_STAB_MASTERED:
			skill = true;
			break;
		case EFFECT_BLESS:
			break;
		case EFFECT_INFINITY_ARROW_IMPROVED:
			skill = true;
			break;
		case EFFECT_BLIND_IMPROVED:
			skill = true;
			break;
		case EFFECT_DRAIN_LIFE_ENHANCED:
			skill = true;
			break;
		case EFFECT_ICE_STORM_ENHANCED:
			skill = true;
			break;
		case EFFECT_EARTH_PRISON:
			skill = true;
			break;
		case EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED:
			skill = true;
			break;
		case EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED:
			skill = true;
			break;
		case EFFECT_SWORD_POWER_IMPROVED:
			skill = true;
			break;
		case EFFECT_SWORD_POWER_ENHANCED:
			skill = true;
			break;
		case EFFECT_SWORD_POWER_MASTERED:
			skill = true;
			break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED:
			skill = true;
			break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED:
			skill = true;
			break;
		case EFFECT_FITNESS_IMPROVED:
			skill = true;
			break;
		case EFFECT_DRAGON_ROAR_ENHANCED:
			skill = true;
			break;
		case EFFECT_CHAIN_DRIVER_ENHANCED:
			skill = true;
			break;
		case EFFECT_POISON_ARROW:
			skill = true;
			break;
		case EFFECT_POISON_ARROW_IMPROVED:
			skill = true;
			break;
		case EFFECT_BLESS_IMPROVED:
			break;
		case EFFECT_LESSER_DAMAGE_IMPROVED:
			skill = true;
			break;
		case EFFECT_LESSER_DEFENSE_IMPROVED:
			skill = true;
			break;
		case EFFECT_FIRE_SLASH_ENHANCED:
			skill = true;
			break;
		case EFFECT_IRON_DEFENSE_IMPROVED:
			skill = true;
			break;
		case EFFECT_BLOOD_HOWLING:
			break;
		case EFFECT_BLOOD_HOWLING_IMPROVED:
			break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_SD:
			break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_MP:
			break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_SPEED:
			break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_HP:
			break;
		case EFFECT_PENTAGRAM_JEWEL_STUN:
			break;
		case EFFECT_PENTAGRAM_JEWEL_SLOW:
			break;
		case EFFECT_TALISMAN_OF_ASCENSION1:
			break;
		case EFFECT_TALISMAN_OF_ASCENSION2:
			skill = true;
			break;
		case EFFECT_TALISMAN_OF_ASCENSION3:
			break;
		case EFFECT_SEAL_OF_ASCENSION3:
			break;
		case EFFECT_MASTER_SEAL_OF_ASCENSION2:
			break;
		case EFFECT_BLESSING_OF_LIGHT:
			break;
		case EFFECT_MASTER_SCROLL_OF_DEFENSE:
			break;
		case EFFECT_MASTER_SCROLL_OF_MAGIC_DAMAGE:
			break;
		case EFFECT_MASTER_SCROLL_OF_LIFE:
			break;
		case EFFECT_MASTER_SCROLL_OF_MANA:
			break;
		case EFFECT_MASTER_SCROLL_OF_DAMAGE:
			break;
		case EFFECT_MASTER_SCROLL_OF_HEALING:
			break;
		case EFFECT_MASTER_SCROLL_OF_BATTLE:
			break;
		case EFFECT_MASTER_SCROLL_OF_STRENGTH:
			break;
		case EFFECT_MASTER_SCROLL_OF_QUICK:
			break;
	}

	return skill;
}

void CCustomStartItem::GiftItem(LPOBJ lpObj)
{
	int regalo = 0;

	if (lpObj->ItemStart >= 1)
	{
		return;
	}

	if (gItemManager.CheckItemInventorySpace(lpObj, 4, 4) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "Your inventory is full");
		return;
	}

	CustomStartItem_INFO* lpInfo = &this->m_CustomStartItemInfo[lpObj->Class];

	for (std::vector<CUSTOM_START_ITEM_INFO>::iterator it = lpInfo->ItemStart.begin(); it != lpInfo->ItemStart.end(); it++)
	{
		int Hours = it->ItemTime * 3600;
		DWORD tiempoitem = (DWORD)time(0) + Hours;

		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { 0xFF,0xFF,0xFF,0xFF,0xFF };

		if (gSocketItemType.CheckSocketItemType(it->ItemIndex) == 1)
		{
			int qtd = gSocketItemType.GetSocketItemMaxSocket(it->ItemIndex);

			ItemSocketOption[0] = (BYTE)((qtd > 0) ? ((it->ItemSocket1 != 255) ? it->ItemSocket1 : 255) : 255);
			ItemSocketOption[1] = (BYTE)((qtd > 1) ? ((it->ItemSocket2 != 255) ? it->ItemSocket2 : 255) : 255);
			ItemSocketOption[2] = (BYTE)((qtd > 2) ? ((it->ItemSocket3 != 255) ? it->ItemSocket3 : 255) : 255);
			ItemSocketOption[3] = (BYTE)((qtd > 3) ? ((it->ItemSocket4 != 255) ? it->ItemSocket4 : 255) : 255);
			ItemSocketOption[4] = (BYTE)((qtd > 4) ? ((it->ItemSocket5 != 255) ? it->ItemSocket5 : 255) : 255);
		}
		regalo++;
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, it->ItemIndex, it->ItemLevel, 0, it->ItemSkill, it->ItemLuck, it->ItemOption, -1, it->ItemExcellent, it->ItemAncient, it->ItemJOH, it->ItemOpEx, ItemSocketOption, 0xFF, tiempoitem);
	}

	CustomStartBuff_INFO * t = &this->m_CustomStartBuffInfo[lpObj->Class];

	for(std::vector<CUSTOM_START_BUFF_INFO>::iterator it = t->BuffStart.begin(); it != t->BuffStart.end();it++)
	{
		if(it->Effect == EFFECT_ELF_BUFFER)
		{
			gEffectManager.AddEffect(lpObj, 0, EFFECT_ELF_BUFFER, it->Time, (gServerInfo.m_ElfBufferDamageConstA+(lpObj->Level/gServerInfo.m_ElfBufferDamageConstB)), (gServerInfo.m_ElfBufferDefenseConstA+(lpObj->Level/gServerInfo.m_ElfBufferDefenseConstB)),0,0);
		}
		else
		{
			gEffectManager.AddEffect(lpObj, (this->CheckSkillEffect(it->Effect) == true)?0:1, it->Effect,(it->Type == 0)?(int)(time(0)+it->Time):it->Time, it->Power1, it->Power2, 0, 0);
		}

		regalo++;
	}

	if( regalo != 0 )
	{
		lpObj->ItemStart += 1;

		GDSaveTheGiftData(lpObj->Index);
	}
}
