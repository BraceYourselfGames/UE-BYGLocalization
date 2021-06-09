// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "BYGLocalizationUIStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "EditorStyleSet.h"
#include "Interfaces/IPluginManager.h"
#include "SlateOptMacros.h"



#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)


TSharedPtr< FSlateStyleSet > FBYGLocalizationUIStyle::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FBYGLocalizationUIStyle::Get() { return StyleSet; }

FName FBYGLocalizationUIStyle::GetStyleSetName()
{
	static FName BYGLocalizationStyleName( TEXT( "BYGLocalizationUIStyle" ) );
	return BYGLocalizationStyleName;
}

void FBYGLocalizationUIStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8( 8.0f, 8.0f );
	const FVector2D Icon16x16( 16.0f, 16.0f );
	const FVector2D Icon20x20( 20.0f, 20.0f );
	const FVector2D Icon40x40( 40.0f, 40.0f );
	const FVector2D Icon64x64( 64.0f, 64.0f );
	const FVector2D Spacer32x4( 32, 1 );

	// Only register once
	if ( StyleSet.IsValid() )
	{
		return;
	}

	StyleSet = MakeShareable( new FSlateStyleSet( GetStyleSetName() ) );

	StyleSet->SetContentRoot( IPluginManager::Get().FindPlugin( TEXT( "BYGLocalization" ) )->GetBaseDir() / TEXT( "Resources" ) );
	StyleSet->SetCoreContentRoot( FPaths::EngineContentDir() / TEXT( "Slate" ) );

	// BYGLocalizationBlock
	StyleSet->Set( "BYGLocalization.TabIcon", new IMAGE_BRUSH( "Icon20", Icon20x20 ) );
	StyleSet->Set( "BYGLocalization.OpenFile", new IMAGE_BRUSH( "OpenFile20", Icon20x20 ) );
	StyleSet->Set( "BYGLocalization.OpenFolder", new IMAGE_BRUSH( "OpenFolder20", Icon20x20 ) );

	const FSlateFontInfo NormalFont = DEFAULT_FONT( "Regular", FCoreStyle::RegularTextSize );


	const FTextBlockStyle NormalText = FEditorStyle::GetWidgetStyle<FTextBlockStyle>( "NormalText" );

	StyleSet->Set( "LocalizationStyle.Title", FTextBlockStyle( NormalText )
		.SetFont( DEFAULT_FONT( "Bold", FCoreStyle::RegularTextSize ) )
		.SetFontSize( 10 )
		.SetColorAndOpacity( FLinearColor( 1.0f, 1.0f, 1.0f ) )
		.SetHighlightColor( FLinearColor( 1.0f, 1.0f, 1.0f ) )
		.SetShadowOffset( FVector2D( 1, 1 ) )
		.SetShadowColorAndOpacity( FLinearColor( 0, 0, 0, 0.9f ) ) );

	StyleSet->Set( "LocalizationProperty.Title", FTextBlockStyle( NormalText )
		.SetFont( DEFAULT_FONT( "Bold", FCoreStyle::RegularTextSize ) )
		.SetFontSize( 10 )
		.SetColorAndOpacity( FLinearColor( 1.0f, 1.0f, 1.0f ) )
		.SetHighlightColor( FLinearColor( 1.0f, 1.0f, 1.0f ) )
		.SetShadowOffset( FVector2D( 1, 1 ) )
		.SetShadowColorAndOpacity( FLinearColor( 0, 0, 0, 0.9f ) ) );


	FSlateStyleRegistry::RegisterSlateStyle( *StyleSet.Get() );
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef DEFAULT_FONT

void FBYGLocalizationUIStyle::Shutdown()
{
	if ( StyleSet.IsValid() )
	{
		FSlateStyleRegistry::UnRegisterSlateStyle( *StyleSet.Get() );
		ensure( StyleSet.IsUnique() );
		StyleSet.Reset();
	}
}

const FSlateBrush* FBYGLocalizationUIStyle::GetBrush( FName PropertyName, const ANSICHAR* Specifier )
{
	return StyleSet->GetBrush( PropertyName, Specifier );
}
