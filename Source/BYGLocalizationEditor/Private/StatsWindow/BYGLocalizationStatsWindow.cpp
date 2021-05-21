// Copyright Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationStatsWindow.h"

//#include "Widgets/Layout/SScrollBox.h"
//#include "Engine/Font.h"
#include "Interfaces/IPluginManager.h"
#include "BYGLocalization/Public/BYGLocalization.h"

#define LOCTEXT_NAMESPACE "BYGLocalization"

SBYGLocalizationStatsWindow::~SBYGLocalizationStatsWindow()
{
}

void SBYGLocalizationStatsWindow::Construct( const FArguments& InArgs )
{
	// Load the data I guess?
	Items.Empty();

	TArray<FBYGLocalizationLocaleBasic> Entries = UBYGLocalization::GetAvailableLocalizations( true );

	for ( const FBYGLocalizationLocaleBasic& Entry : Entries )
	{
		TSharedRef<FBYGLocalizationStatEntry> NewItem = FBYGLocalizationStatEntry::Create();
		NewItem->LocaleCode = Entry.LocaleCode;
		NewItem->Language = Entry.Language;
		NewItem->Path = Entry.FilePath;

		// Get stats
		FBYGLocalizationLocale Data = UBYGLocalization::GetLocalizationData( FPaths::Combine( FPaths::ProjectContentDir(), Entry.FilePath ) );
		NewItem->NormalEntries = Data.StatusCounts.FindOrAdd( EBYGLocalizationStatus::None, 0 );
		NewItem->NewEntries = Data.StatusCounts.FindOrAdd( EBYGLocalizationStatus::New, 0 );
		NewItem->ModifiedEntries = Data.StatusCounts.FindOrAdd( EBYGLocalizationStatus::Modified, 0 );
		NewItem->DeprecatedEntries = Data.StatusCounts.FindOrAdd( EBYGLocalizationStatus::Deprecated, 0 );
		NewItem->TotalEntries = NewItem->NormalEntries + NewItem->NewEntries + NewItem->ModifiedEntries + NewItem->DeprecatedEntries;

		Items.Add( NewItem );
	}

	ChildSlot
	[
		SNew( SBorder )
		.Padding( 3 )
		.BorderImage( FEditorStyle::GetBrush( "ToolPanel.GroupBorder" ) )
		[
			// The list view being tested
			SAssignNew( StatsList, SListView< TSharedPtr<FBYGLocalizationStatEntry> > )
			.ItemHeight( 24 )
			.ListItemsSource( &Items )
			.OnGenerateRow( this, &SBYGLocalizationStatsWindow::OnGenerateWidgetForList )
			//.OnContextMenuOpening( this, &STableViewTesting::GetListContextMenu )
			.SelectionMode( ESelectionMode::None )
			.HeaderRow
			(
				SNew( SHeaderRow )
				+ SHeaderRow::Column( "LocaleCode" ).DefaultLabel( LOCTEXT( "LocaleColumn", "Locale" ) )
				+ SHeaderRow::Column( "Language" ).DefaultLabel( LOCTEXT( "LanguageColumn", "Language" ) )
				+ SHeaderRow::Column( "Path" ).DefaultLabel( LOCTEXT( "PathColumn", "Path" ) )
				+ SHeaderRow::Column( "Normal" ).DefaultLabel( LOCTEXT( "NormalColumn", "Normal" ) )
				+ SHeaderRow::Column( "New" ).DefaultLabel( LOCTEXT( "NewColumn", "New" ) )
				+ SHeaderRow::Column( "Modified" ).DefaultLabel( LOCTEXT( "ModifiedColumn", "Modified" ) )
				+ SHeaderRow::Column( "Deprecated" ).DefaultLabel( LOCTEXT( "DeprecatedColumn", "Deprecated" ) )
				+ SHeaderRow::Column( "Total" ).DefaultLabel( LOCTEXT( "TotalColumn", "Total" ) )
			)
		]
	];
}


TSharedRef<ITableRow> SBYGLocalizationStatsWindow::OnGenerateWidgetForList( TSharedPtr<FBYGLocalizationStatEntry> InItem, const TSharedRef<STableViewBase>& OwnerTable )
{
	return SNew( SBYGEntryTableRow, OwnerTable )
		// Triggered when a user is attempting to drag, so initiate a DragDropOperation.
		//.OnDragDetected(this, &STableViewTesting::OnDragDetected_Handler, TWeakPtr<FBYGLocalizationStatEntry>(InItem))

		// Given a hovered drop zone (above, onto, or below) respond with a zone where we would actually drop the item.
		// Respond with an un-set TOptional<EItemDropZone> if we cannot drop here at all.
		//.OnCanAcceptDrop(this, &STableViewTesting::OnCanAcceptDrop_Handler)

		// Actually perform the drop into the given drop zone.
		//.OnAcceptDrop(this, &STableViewTesting::OnAcceptDrop_Handler)

		.ItemToEdit( InItem );
}

#undef LOCTEXT_NAMESPACE
