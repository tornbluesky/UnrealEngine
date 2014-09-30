// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WrapBox.generated.h"

/**
* A vertical box widget is a layout panel allowing child widgets to be automatically laid out
* vertically.
*/
UCLASS(ClassGroup=UserInterface)
class UMG_API UWrapBox : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** The inner slot padding goes between slots sharing borders */
	UPROPERTY(EditDefaultsOnly, Category="Conetnt Layout")
	FVector2D InnerSlotPadding;

public:

	/**  */
	UFUNCTION(BlueprintCallable, Category="Panel")
	UWrapBoxSlot* AddChildWrapBox(UWidget* Content);

#if WITH_EDITOR
	// UWidget interface
	virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetPaletteCategory() override;
	// End UWidget interface
#endif

protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

protected:

	TSharedPtr<class SWrapBox> MyWrapBox;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
