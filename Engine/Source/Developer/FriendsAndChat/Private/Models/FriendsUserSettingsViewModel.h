// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class FFriendsUserSettingsViewModel
	: public TSharedFromThis<FFriendsUserSettingsViewModel>
{
public:
	virtual ~FFriendsUserSettingsViewModel() {}
	virtual void HandleCheckboxStateChanged(ESlateCheckBoxState::Type NewState, EUserSettngsType::Type OptionType) = 0;
	virtual void EnumerateUserSettings(TArray<EUserSettngsType::Type>& UserSettings) = 0;
	virtual ESlateCheckBoxState::Type GetOptionCheckState(EUserSettngsType::Type Option) const = 0;
};

/**
 * Creates the implementation for an FFriendsUserSettingsViewModel.
 *
 * @return the newly created FFriendsUserSettingsViewModel implementation.
 */
FACTORY(TSharedRef< FFriendsUserSettingsViewModel >, FFriendsUserSettingsViewModel,
	const TSharedRef<class FFriendsAndChatManager>& FFriendsAndChatManager );