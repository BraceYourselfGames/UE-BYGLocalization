// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatsWindow.h"

#include "Interfaces/IPluginManager.h"

#include "Widgets/Views/SExpanderArrow.h"

#include "BYGLocalization/Public/BYGLocalization.h"
#include "BYGLocalizationEditor/Private/BYGLocalizationUIStyle.h"
#include "BYGLocalizationModule.h"

#define LOCTEXT_NAMESPACE "BYGLocalization"

SBYGLocalizationStatsWindow::~SBYGLocalizationStatsWindow()
{
	CleanupThreads();
}

void SBYGLocalizationStatsWindow::Construct( const FArguments& InArgs )
{
	ChildSlot
	[
		SNew( SBorder )
		.Padding( 3 )
		.BorderImage( FEditorStyle::GetBrush( "ToolPanel.GroupBorder" ) )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew( SButton )
					.ButtonStyle( FEditorStyle::Get(), "FlatButton.Default" )
					.TextStyle( FEditorStyle::Get(), "FlatButton.DefaultTextStyle" )
					.OnClicked( this, &SBYGLocalizationStatsWindow::RefreshAll )
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.VAlign( VAlign_Center )
						.Padding( FMargin( 0, 0, 2, 0 ) )
						.AutoWidth()
						[
							SNew( STextBlock )
							.Font( FEditorStyle::Get().GetFontStyle( "FontAwesome.11" ) )
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text( FText::FromString( FString( TEXT( "\xf01e" ) ) ) )
						]
						+ SHorizontalBox::Slot()
						.VAlign( VAlign_Center )
						.AutoWidth()
						[
							SNew( STextBlock )
							.Text( LOCTEXT( "RefreshAll", "Refresh All" ) )
						]
					]
				]
				#if 0
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew( SButton )
					.OnClicked( this, &SBYGLocalizationStatsWindow::CancelAll )
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew( STextBlock )
							.Font( FEditorStyle::Get().GetFontStyle( "FontAwesome.11" ) )
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
							.Text( FText::FromString( FString( TEXT( "\xf01e" ) ) ) /*fa-folder-open*/ )
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew( STextBlock )
							.Text( LOCTEXT( "CancelAll", "Cancel" ) )
						]
					]
				]
				#endif
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew( StatusThrobber, SCircularThrobber )
					.Visibility( EVisibility::Hidden )
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight( 1.0 )
			[
				// The list view being tested
				SAssignNew( StatsList, SListView< TSharedPtr<FBYGLocalizationStatEntry> > )
				.ItemHeight( 24 )
				.ListItemsSource( &Items )
				.OnGenerateRow( this, &SBYGLocalizationStatsWindow::OnGenerateWidgetForList )
				.OnContextMenuOpening( this, &SBYGLocalizationStatsWindow::GetListContextMenu )
				.OnMouseButtonDoubleClick( this, &SBYGLocalizationStatsWindow::OnDoubleClicked )
				.SelectionMode( ESelectionMode::Multi )
				.HeaderRow
				(
					SNew( SHeaderRow )
					+ SHeaderRow::Column( "PrimaryLanguage" ).DefaultLabel( LOCTEXT( "PrimaryLanguageColumn", "Primary" ) ).ToolTipText( LOCTEXT( "PrimaryLanguageColumnTooltip", "Primary Language" ) ).FixedWidth( 60 )
					+ SHeaderRow::Column( "LocaleCode" ).DefaultLabel( LOCTEXT( "LocaleColumn", "Locale Code" ) ).FixedWidth( 80 )
					+ SHeaderRow::Column( "Language" ).DefaultLabel( LOCTEXT( "LanguageColumn", "Language" ) )
					+ SHeaderRow::Column( "Path" ).DefaultLabel( LOCTEXT( "PathColumn", "Path" ) )
					+ SHeaderRow::Column( "Normal" ).DefaultLabel( LOCTEXT( "NormalColumn", "Normal" ) ).HAlignCell( HAlign_Right ).HAlignHeader( HAlign_Right ).FixedWidth( 70 )
					+ SHeaderRow::Column( "New" ).DefaultLabel( LOCTEXT( "NewColumn", "New" ) ).HAlignCell( HAlign_Right ).HAlignHeader( HAlign_Right ).FixedWidth( 70 )
					+ SHeaderRow::Column( "Modified" ).DefaultLabel( LOCTEXT( "ModifiedColumn", "Modified" ) ).HAlignCell( HAlign_Right ).HAlignHeader( HAlign_Right ).FixedWidth( 70 )
					+ SHeaderRow::Column( "Deprecated" ).DefaultLabel( LOCTEXT( "DeprecatedColumn", "Deprecated" ) ).HAlignCell( HAlign_Right ).HAlignHeader( HAlign_Right ).FixedWidth( 70 )
					+ SHeaderRow::Column( "Total" ).DefaultLabel( LOCTEXT( "TotalColumn", "Total" ) ).HAlignCell( HAlign_Right ).HAlignHeader( HAlign_Right ).FixedWidth( 70 )
					+ SHeaderRow::Column( "Status" ).DefaultLabel( LOCTEXT( "StatusColumn", "Status" ) )
				)
			]
		]
	];

	RefreshAll();
}



TSharedRef<ITableRow> SBYGLocalizationStatsWindow::OnGenerateWidgetForList( TSharedPtr<FBYGLocalizationStatEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	return SNew( SBYGEntryTableRow, OwnerTable )
		.ItemToEdit( InItem );
}

TSharedPtr<SWidget> SBYGLocalizationStatsWindow::GetListContextMenu()
{
	FMenuBuilder MenuBuilder( true, NULL, TSharedPtr<FExtender>(), false, &FCoreStyle::Get() );
	{
		MenuBuilder.BeginSection( "FileActions", LOCTEXT( "FileActions", "File Actions" ) );
		{
			FUIAction Action_OpenFile(
				FExecuteAction::CreateRaw( this, &SBYGLocalizationStatsWindow::OpenFile ) );
			MenuBuilder.AddMenuEntry(
				LOCTEXT( "OpenFile", "Open File" ),
				LOCTEXT( "OpenFileTooltip", "Open the file with the default editor." ),
				FSlateIcon( FBYGLocalizationUIStyle::GetStyleSetName(), "BYGLocalization.OpenFile" ),
				Action_OpenFile,
				NAME_None,
				EUserInterfaceActionType::Button );

			FUIAction Action_OpenFolder(
				FExecuteAction::CreateRaw( this, &SBYGLocalizationStatsWindow::OpenFolder ) );
			MenuBuilder.AddMenuEntry(
				LOCTEXT( "OpenFolder", "Open Folder" ),
				LOCTEXT( "OpenFolderTooltip", "Open the containing folder." ),
				FSlateIcon(FBYGLocalizationUIStyle::GetStyleSetName(), "BYGLocalization.OpenFolder"),
				Action_OpenFolder,
				NAME_None,
				EUserInterfaceActionType::Button );

				#if 0
			FUIAction Action_RefreshFile(
				FExecuteAction::CreateRaw( this, &SBYGLocalizationStatsWindow::RefreshFile ) );
			MenuBuilder.AddMenuEntry(
				LOCTEXT( "RefreshFile", "Refresh File" ),
				LOCTEXT( "RefreshFileTooltip", "Refresh the file with the default editor." ),
				FSlateIcon( FBYGLocalizationUIStyle::GetStyleSetName(), "BYGLocalization.RefreshFile" ),
				Action_RefreshFile,
				NAME_None,
				EUserInterfaceActionType::Button );
				#endif
		}
		MenuBuilder.EndSection();

	}

	return  MenuBuilder.MakeWidget();
#if 0
	return
		SNew( SBorder )
		.Padding( 3 )
		.BorderImage( FEditorStyle::GetBrush( "DetailsView.AdvancedDropdownBorder" ) )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew( SButton )
				.ButtonStyle( FEditorStyle::Get(), "FlatButton" )
				.OnClicked( this, &SBYGLocalizationStatsWindow::OpenFolder )
				[
					SNew( SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew( STextBlock )
						.Font( FEditorStyle::Get().GetFontStyle( "FontAwesome.11" ) )
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Text( FText::FromString( FString( TEXT( "\xf07c" ) ) ) /*fa-folder-open*/ )
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew( STextBlock )
						.Text( LOCTEXT( "OpenFolder", "Open folder" ) )
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew( SButton )
				.ButtonStyle( FEditorStyle::Get(), "FlatButton" )
				.OnClicked( this, &SBYGLocalizationStatsWindow::OpenFile )
				[
					SNew( SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew( STextBlock )
						.Font( FEditorStyle::Get().GetFontStyle( "FontAwesome.11" ) )
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Text( FText::FromString( FString( TEXT( "\xf15b" ) ) ) /*fa-file*/ )
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew( STextBlock )
						.Text( LOCTEXT( "OpenFile", "Open file in default editor" ) )
					]
				]
			]
		];

		#endif
}

void SBYGLocalizationStatsWindow::CleanupThreads()
{
	ParseFileRunnable.Reset();
}

FReply SBYGLocalizationStatsWindow::RefreshAll()
{
	CleanupThreads();

	// Reload all

	// Load the data I guess?
	Items.Empty();

	TArray<FBYGLocaleInfo> Entries = FBYGLocalizationModule::Get().GetLocalization()->GetAvailableLocalizations();

	TArray<FString> Paths;

	for ( const FBYGLocaleInfo& Entry : Entries )
	{
		const FString FullPath = FPaths::Combine( FPaths::ProjectContentDir(), Entry.FilePath );
		Paths.Add( FullPath );

		TSharedRef<FBYGLocalizationStatEntry> NewItem = FBYGLocalizationStatEntry::Create();
		NewItem->LocaleCode = Entry.LocaleCode;
		NewItem->Language = Entry.LocalizedName;
		NewItem->Path = FPaths::Combine( FPaths::ProjectContentDir(), Entry.FilePath );
		NewItem->bIsRefreshing = true;
		Items.Add( NewItem );
	}

	ParseFileRunnable = MakeShareable( new FBYGParseFileRunnable( Paths ) );
	if ( ParseFileRunnable )
	{
		ParseFileRunnable->OnGetStatsCompleteSignature.BindRaw( this, &SBYGLocalizationStatsWindow::OnFileParseComplete );
	}

	return FReply::Handled();
}

void SBYGLocalizationStatsWindow::OnFileParseComplete( const FString& Path, const BYGLocStats& LocStats )
{
	// Find the data for this one
	for ( TSharedPtr<FBYGLocalizationStatEntry> Entry : Items )
	{
		if ( !Entry.IsValid() )
			continue;

		if ( Entry->Path == Path )
		{
			Entry->bIsRefreshing = false;
			Entry->NormalEntries = LocStats[ EBYGLocEntryStatus::None ];
			Entry->NewEntries = LocStats[ EBYGLocEntryStatus::New ];
			Entry->ModifiedEntries = LocStats[ EBYGLocEntryStatus::Modified ];
			Entry->DeprecatedEntries = LocStats[ EBYGLocEntryStatus::Deprecated ];
			Entry->TotalEntries = LocStats[ EBYGLocEntryStatus::None ] + LocStats[ EBYGLocEntryStatus::New ] + LocStats[ EBYGLocEntryStatus::Modified ];
			break;
		}
	}
}


FReply SBYGLocalizationStatsWindow::CancelAll()
{
	return FReply::Handled();
}

void SBYGLocalizationStatsWindow::OpenFolder()
{
	TArray<TSharedPtr<FBYGLocalizationStatEntry>> SelectedItems;
	StatsList->GetSelectedItems( SelectedItems );

	for ( int32 i = 0; i < SelectedItems.Num(); ++i )
	{

		const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead( *FPaths::GetPath( SelectedItems[ i ]->Path ) );
		FPlatformProcess::ExploreFolder( *AbsolutePath );
	}

	//return FReply::Unhandled();
}

void SBYGLocalizationStatsWindow::OpenFile()
{
	TArray<TSharedPtr<FBYGLocalizationStatEntry>> SelectedItems;
	StatsList->GetSelectedItems( SelectedItems );

	for ( int32 i = 0; i < SelectedItems.Num(); ++i )
	{
		const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead( *SelectedItems[ i ]->Path );
		FPlatformProcess::LaunchFileInDefaultExternalApplication( *AbsolutePath );
	}
}

void SBYGLocalizationStatsWindow::OnDoubleClicked( TSharedPtr<FBYGLocalizationStatEntry> Entry )
{
	if ( Entry.IsValid() )
	{
		const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead( *Entry->Path );
		FPlatformProcess::LaunchFileInDefaultExternalApplication( *AbsolutePath );
	}
}

void SBYGLocalizationStatsWindow::RefreshFile()
{
	TArray<TSharedPtr<FBYGLocalizationStatEntry>> SelectedItems;
	StatsList->GetSelectedItems( SelectedItems );

	for ( int32 i = 0; i < SelectedItems.Num(); ++i )
	{

	}
}

#if 0
FReply SBYGLocalizationStatsWindow::RememberExpansion()
{
	StoredExpandedItems.Empty();
	StatsList->GetExpandedItems( StoredExpandedItems );
	return FReply::Handled();
}

FReply SBYGLocalizationStatsWindow::CollapseAll()
{
	TreeBeingTested->ClearExpandedItems();
	TreeBeingTested->RequestTreeRefresh();
	return FReply::Handled();
}

FReply SBYGLocalizationStatsWindow::RestoreExpansion()
{
	for ( const TSharedPtr<FTestData>& Item : StoredExpandedItems )
	{
		TreeBeingTested->SetItemExpansion( Item, true );
	}
	TreeBeingTested->RequestTreeRefresh();
	return FReply::Handled();
}
#endif

void SBYGEntryTableRow::ToggleExpansion()
{
	bIsExpanded = !bIsExpanded;
	CategoryContent->SetVisibility( bIsExpanded ? EVisibility::Visible : EVisibility::Collapsed );
}

const FSlateBrush* SBYGEntryTableRow::GetBackgroundImage() const
{
	if ( IsHovered() )
	{
		return bIsExpanded ? FEditorStyle::GetBrush( "DetailsView.CategoryTop_Hovered" ) : FEditorStyle::GetBrush( "DetailsView.CollapsedCategory_Hovered" );
	}
	else
	{
		return bIsExpanded ? FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) : FEditorStyle::GetBrush( "DetailsView.CollapsedCategory" );
	}
}

#undef LOCTEXT_NAMESPACE

