// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UMGEditorPrivatePCH.h"
#include "SWidgetDetailsView.h"

#include "BlueprintEditor.h"
#include "IDetailsView.h"
#include "Kismet2NameValidators.h"
#include "ISequencer.h"
#include "Animation/UMGDetailKeyframeHandler.h"
#include "EditorClassUtils.h"

#include "Customizations/SlateBrushCustomization.h"

#include "CanvasSlotCustomization.h"
#include "HorizontalAlignmentCustomization.h"
#include "VerticalAlignmentCustomization.h"
#include "SlateChildSizeCustomization.h"
#include "TextJustifyCustomization.h"

#define LOCTEXT_NAMESPACE "UMG"

void SWidgetDetailsView::Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> InBlueprintEditor)
{
	BlueprintEditor = InBlueprintEditor;

	// Create a property view
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FNotifyHook* NotifyHook = this;
	FDetailsViewArgs DetailsViewArgs( /*bUpdateFromSelection=*/ false, /*bLockable=*/ false, /*bAllowSearch=*/ true, /*bObjectsUseNameArea=*/ true, /*bHideSelectionTip=*/ true, /*InNotifyHook=*/ NotifyHook, /*InSearchInitialKeyFocus=*/ false, /*InViewIdentifier=*/ NAME_None);

	// We hide the actor name area because we're providing our own.
	DetailsViewArgs.bHideActorNameArea = true;

	PropertyView = EditModule.CreateDetailView(DetailsViewArgs);

	// Create a handler for keyframing via the details panel
	TSharedRef<IDetailKeyframeHandler> KeyframeHandler = MakeShareable( new FUMGDetailKeyframeHandler( InBlueprintEditor ) );
	PropertyView->SetKeyframeHandler( KeyframeHandler );

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 6)
		[
			SNew(SHorizontalBox)
			.Visibility(this, &SWidgetDetailsView::GetNameAreaVisibility)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 3, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SWidgetDetailsView::GetNameIcon)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 6, 0)
			[
				SNew(SBox)
				.WidthOverride(200.0f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(NameTextBox, SEditableTextBox)
					.SelectAllTextWhenFocused(true)
					.HintText(LOCTEXT("Name", "Name"))
					.Text(this, &SWidgetDetailsView::GetNameText)
					.OnTextChanged(this, &SWidgetDetailsView::HandleNameTextChanged)
					.OnTextCommitted(this, &SWidgetDetailsView::HandleNameTextCommitted)
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				//.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
				.IsChecked(this, &SWidgetDetailsView::GetIsVariable)
				.OnCheckStateChanged(this, &SWidgetDetailsView::HandleIsVariableChanged)
				.Padding(FMargin(3,1,3,1))
				[
					SNew(STextBlock)
					//.Font(FEditorStyle::GetFontStyle("BoldFont"))
					.Text(LOCTEXT("IsVariable", "Is Variable"))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(15,0,0,0)
			[
				SAssignNew(ClassLinkArea, SBox)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			PropertyView.ToSharedRef()
		]
	];

	BlueprintEditor.Pin()->OnSelectedWidgetsChanging.AddRaw(this, &SWidgetDetailsView::OnEditorSelectionChanging);
	BlueprintEditor.Pin()->OnSelectedWidgetsChanged.AddRaw(this, &SWidgetDetailsView::OnEditorSelectionChanged);

	RegisterCustomizations();
}

SWidgetDetailsView::~SWidgetDetailsView()
{
	if ( BlueprintEditor.IsValid() )
	{
		BlueprintEditor.Pin()->OnSelectedWidgetsChanging.RemoveAll(this);
		BlueprintEditor.Pin()->OnSelectedWidgetsChanged.RemoveAll(this);
	}

	// Unregister the property type layouts
	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);

	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("PanelSlot"), nullptr, PropertyView);
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("EHorizontalAlignment"), nullptr, PropertyView);
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("EVerticalAlignment"), nullptr, PropertyView);
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("SlateChildSize"), nullptr, PropertyView);
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("SlateBrush"), nullptr, PropertyView);
	PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("ETextJustify"), nullptr, PropertyView);
}

void SWidgetDetailsView::RegisterCustomizations()
{
	PropertyView->RegisterInstancedCustomPropertyLayout(UWidget::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FBlueprintWidgetCustomization::MakeInstance, BlueprintEditor.Pin().ToSharedRef(), BlueprintEditor.Pin()->GetBlueprintObj()));

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);

	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("PanelSlot"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCanvasSlotCustomization::MakeInstance, BlueprintEditor.Pin()->GetBlueprintObj()), nullptr, PropertyView);
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("EHorizontalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHorizontalAlignmentCustomization::MakeInstance), nullptr, PropertyView);
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("EVerticalAlignment"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVerticalAlignmentCustomization::MakeInstance), nullptr, PropertyView);
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("SlateChildSize"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSlateChildSizeCustomization::MakeInstance), nullptr, PropertyView);
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("SlateBrush"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSlateBrushStructCustomization::MakeInstance, false), nullptr, PropertyView);
	PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("ETextJustify"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FTextJustifyCustomization::MakeInstance), nullptr, PropertyView);
}

void SWidgetDetailsView::OnEditorSelectionChanging()
{
	ClearFocusIfOwned();
}

void SWidgetDetailsView::OnEditorSelectionChanged()
{
	SelectedObjects.Empty();
	PropertyView->SetObjects(SelectedObjects);

	TSet< FWidgetReference > SelectedWidgets = BlueprintEditor.Pin()->GetSelectedWidgets();
	if ( SelectedWidgets.Num() > 0 )
	{
		for ( FWidgetReference& WidgetRef : SelectedWidgets )
		{
			SelectedObjects.Add(WidgetRef.GetPreview());
		}
	}

	TSet< TWeakObjectPtr<UObject> > Selection = BlueprintEditor.Pin()->GetSelectedObjects();
	for ( TWeakObjectPtr<UObject> Selected : Selection )
	{
		if ( UObject* S = Selected.Get() )
		{
			SelectedObjects.Add(S);
		}
	}

	if ( SelectedObjects.Num() == 1 && SelectedObjects[0].IsValid() )
	{
		ClassLinkArea->SetContent(FEditorClassUtils::GetSourceLink(SelectedObjects[0]->GetClass(), TWeakObjectPtr<UObject>()));
	}
	else
	{
		ClassLinkArea->SetContent(SNullWidget::NullWidget);
	}

	const bool bForceRefresh = false;
	PropertyView->SetObjects(SelectedObjects, bForceRefresh);
}

void SWidgetDetailsView::ClearFocusIfOwned()
{
	static bool bIsReentrant = false;
	if ( !bIsReentrant )
	{
		bIsReentrant = true;
		// When the selection is changed, we may be potentially actively editing a property,
		// if this occurs we need need to immediately clear keyboard focus
		if ( FSlateApplication::Get().HasFocusedDescendants(AsShared()) )
		{
			FSlateApplication::Get().ClearKeyboardFocus(EKeyboardFocusCause::Mouse);
		}
		bIsReentrant = false;
	}
}

EVisibility SWidgetDetailsView::GetNameAreaVisibility() const
{
	if ( SelectedObjects.Num() == 1 )
	{
		UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
		if ( Widget && !Widget->HasAnyFlags(RF_ClassDefaultObject) )
		{
			return EVisibility::Visible;
		}
	}
	
	return EVisibility::Collapsed;
}

const FSlateBrush* SWidgetDetailsView::GetNameIcon() const
{
	if ( SelectedObjects.Num() == 1 )
	{
		UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
		if ( Widget )
		{
			return Widget->GetEditorIcon();
		}
	}

	return NULL;
}

FText SWidgetDetailsView::GetNameText() const
{
	if ( SelectedObjects.Num() == 1 )
	{
		UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
		if ( Widget )
		{
			return FText::FromName(Widget->GetFName());
		}
	}
	
	return FText::GetEmpty();
}

void SWidgetDetailsView::HandleNameTextChanged(const FText& Text)
{
	FText OutErrorMessage;
	if ( !HandleVerifyNameTextChanged(Text, OutErrorMessage) )
	{
		NameTextBox->SetError(OutErrorMessage);
	}
	else
	{
		NameTextBox->SetError(FText::GetEmpty());
	}
}

bool SWidgetDetailsView::HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	if ( SelectedObjects.Num() == 1 )
	{
		UWidget* PreviewWidget = Cast<UWidget>(SelectedObjects[0].Get());

		FString NewName = InText.ToString();

		UWidgetBlueprint* Blueprint = BlueprintEditor.Pin()->GetWidgetBlueprintObj();
		UWidget* TemplateWidget = Blueprint->WidgetTree->FindWidget( FName(*NewName) );

		bool bIsSameWidget = false;
		if ( TemplateWidget != NULL )
		{
			if ( BlueprintEditor.Pin()->GetReferenceFromTemplate( TemplateWidget ).GetPreview() != PreviewWidget )
			{
				OutErrorMessage = LOCTEXT("ExistingWidgetName", "Existing Widget Name");
				return false;
			}
			else
			{
				bIsSameWidget = true;
			}
		}

		FKismetNameValidator Validator(Blueprint);

		const bool bUniqueNameForVariable = ( EValidatorResult::Ok == Validator.IsValid(NewName) );

		if ( !bUniqueNameForVariable && !bIsSameWidget )
		{
			OutErrorMessage = LOCTEXT("ExistingVariableName", "Existing Variable Name");
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

void SWidgetDetailsView::HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	static bool IsReentrant = false;

	if ( !IsReentrant )
	{
		IsReentrant = true;
		if ( SelectedObjects.Num() == 1 )
		{
			FText DummyText;
			if ( HandleVerifyNameTextChanged(Text, DummyText) )
			{
				UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
				FWidgetBlueprintEditorUtils::RenameWidget(BlueprintEditor.Pin().ToSharedRef(), Widget->GetFName(), FName(*Text.ToString()));
			}
		}
		IsReentrant = false;
	}
}

ESlateCheckBoxState::Type SWidgetDetailsView::GetIsVariable() const
{
	if ( SelectedObjects.Num() == 1 )
	{
		UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
		if ( Widget )
		{
			return Widget->bIsVariable ? ESlateCheckBoxState::Checked : ESlateCheckBoxState::Unchecked;
		}
	}

	return ESlateCheckBoxState::Unchecked;
}

void SWidgetDetailsView::HandleIsVariableChanged(ESlateCheckBoxState::Type CheckState)
{
	if ( SelectedObjects.Num() == 1 )
	{
		TSharedPtr<FWidgetBlueprintEditor> BPEditor = BlueprintEditor.Pin();

		UWidget* Widget = Cast<UWidget>(SelectedObjects[0].Get());
		UWidgetBlueprint* Blueprint = BlueprintEditor.Pin()->GetWidgetBlueprintObj();
		
		FWidgetReference WidgetRef = BPEditor->GetReferenceFromTemplate(Blueprint->WidgetTree->FindWidget(Widget->GetFName()));
		if ( WidgetRef.IsValid() )
		{
			UWidget* Template = WidgetRef.GetTemplate();
			UWidget* Preview = WidgetRef.GetPreview();

			const FScopedTransaction Transaction(LOCTEXT("VariableToggle", "Variable Toggle"));
			Template->Modify();
			Preview->Modify();

			Template->bIsVariable = Preview->bIsVariable = CheckState == ESlateCheckBoxState::Checked ? true : false;

			// Refresh references and flush editors
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		}
	}
}

void SWidgetDetailsView::NotifyPreChange(FEditPropertyChain* PropertyAboutToChange)
{
	// During auto-key do not migrate values
	if( !BlueprintEditor.Pin()->GetSequencer()->IsAutoKeyEnabled() )
	{
		TSharedPtr<FWidgetBlueprintEditor> Editor = BlueprintEditor.Pin();

		const bool bIsModify = true;
		Editor->MigrateFromChain(PropertyAboutToChange, bIsModify);
	}
}

void SWidgetDetailsView::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged)
{
	const static FName DesignerRebuildName("DesignerRebuild");

	if ( PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && !BlueprintEditor.Pin()->GetSequencer()->IsAutoKeyEnabled() )
	{
		TSharedPtr<FWidgetBlueprintEditor> Editor = BlueprintEditor.Pin();

		const bool bIsModify = false;
		Editor->MigrateFromChain(PropertyThatChanged, bIsModify);
	}

	// If the property that changed is marked as "DesignerRebuild" we invalidate
	// the preview.
	if ( PropertyChangedEvent.Property->GetBoolMetaData(DesignerRebuildName) )
	{
		BlueprintEditor.Pin()->InvalidatePreview();
	}
}

#undef LOCTEXT_NAMESPACE
