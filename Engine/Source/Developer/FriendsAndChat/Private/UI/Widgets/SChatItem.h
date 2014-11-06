// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class SChatItem : public SUserWidget
{
public:
	SLATE_USER_ARGS(SChatItem)
	{ }
	SLATE_ARGUMENT( const FFriendsAndChatStyle*, FriendStyle )
	SLATE_ARGUMENT(SMenuAnchor::EMethod, Method)
	SLATE_END_ARGS()

	/**
	 * Constructs the widget.
	 *
	 * @param InArgs The Slate argument list.
	 */
	virtual void Construct(const FArguments& InArgs, const TSharedRef<class FChatItemViewModel>& ViewModel) = 0;
};
