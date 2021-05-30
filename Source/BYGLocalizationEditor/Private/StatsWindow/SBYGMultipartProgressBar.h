// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Misc/Attribute.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "SBYGMultipartProgressBar.generated.h"

class FActiveTimerHandle;
class FPaintArgs;
class FSlateWindowElementList;

/**
 * BYGSMultipartProgressBar Fill Type
 */
UENUM( BlueprintType )
namespace EBYGMultipartProgressBarFillType
{
	enum Type
	{
		// will fill up from the left side to the right
		LeftToRight,
		// will fill up from the right side to the left side
		RightToLeft,
		// will fill up from the center to the outer edges
		FillFromCenter,
		// will fill up from the top to the the bottom
		TopToBottom,
		// will fill up from the bottom to the the top
		BottomToTop,
	};
}

/** A progress bar widget.*/
class SBYGMultipartProgressBar : public SLeafWidget
{

public:
	SLATE_BEGIN_ARGS( SBYGMultipartProgressBar )
		: _Style( &FCoreStyle::Get().GetWidgetStyle<FProgressBarStyle>( "ProgressBar" ) )
		, _BarFillType( EBYGMultipartProgressBarFillType::LeftToRight )
		, _Percents( TArray<float>() )
		, _FillColorAndOpacities( TArray<FSlateColor>() )
		, _BorderPadding( FVector2D( 1, 0 ) )
		, _BackgroundImage( nullptr )
		, _FillImage( nullptr )
		, _MarqueeImage( nullptr )
		, _MarqueeScrollSpeed( 1.0f )
		, _RefreshRate( 2.0f )
	{}

	/** Style used for the progress bar */
	SLATE_STYLE_ARGUMENT( FProgressBarStyle, Style )

		/** Defines if this progress bar fills Left to right or right to left*/
		SLATE_ARGUMENT( EBYGMultipartProgressBarFillType::Type, BarFillType )

		/** Used to determine the fill position of the progress bar ranging 0..1 */
		SLATE_ATTRIBUTE( TArray<float>, Percents )

		/** Fill Color and Opacity */
		SLATE_ATTRIBUTE( TArray<FSlateColor>, FillColorAndOpacities )

		/** Border Padding around fill bar */
		SLATE_ATTRIBUTE( FVector2D, BorderPadding )

		/** The brush to use as the background of the progress bar */
		SLATE_ARGUMENT( const FSlateBrush*, BackgroundImage )

		/** The brush to use as the fill image */
		SLATE_ARGUMENT( const FSlateBrush*, FillImage )

		/** The brush to use as the marquee image */
		SLATE_ARGUMENT( const FSlateBrush*, MarqueeImage )

		/** The brush to use as the marquee image */
		SLATE_ARGUMENT( float, MarqueeScrollSpeed )

		/** Rate at which this widget is ticked when sleeping in seconds */
		SLATE_ARGUMENT( float, RefreshRate )

		SLATE_END_ARGS()

		/**
		 * Construct the widget
		 *
		 * @param InArgs   A declaration from which to construct the widget
		 */
		void Construct( const FArguments& InArgs );

	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;
	virtual FVector2D ComputeDesiredSize( float ) const override;
	virtual bool ComputeVolatility() const override;

	/** See attribute Percent */
	void SetPercents( TAttribute< TArray<float> > InPercents );

	/** See attribute Style */
	void SetStyle( const FProgressBarStyle* InStyle );

	/** See attribute BarFillType */
	void SetBarFillType( EBYGMultipartProgressBarFillType::Type InBarFillType );

	/** See attribute SetFillColorAndOpacity */
	void SetFillColorAndOpacities( TAttribute< TArray<FSlateColor > > InFillColorAndOpacities );

	/** See attribute BorderPadding */
	void SetBorderPadding( TAttribute< FVector2D > InBorderPadding );

	/** See attribute BackgroundImage */
	void SetBackgroundImage( const FSlateBrush* InBackgroundImage );

	/** See attribute FillImage */
	void SetFillImage( const FSlateBrush* InFillImage );

	/** See attribute MarqueeImage */
	void SetMarqueeImage( const FSlateBrush* InMarqueeImage );

	void SetShowMarquee( bool bShowMarquee );

	void SetMarqueeScrollSpeed( float InMarqueeScrollSpeed );

private:

	/** Controls the speed at which the widget is ticked when in slate sleep mode */
	void SetActiveTimerTickRate( float TickRate );

	/** Widgets active tick */
	EActiveTimerReturnType ActiveTick( double InCurrentTime, float InDeltaTime );

	/** Gets the current background image. */
	const FSlateBrush* GetBackgroundImage() const;
	/** Gets the current fill image */
	const FSlateBrush* GetFillImage() const;
	/** Gets the current marquee image */
	const FSlateBrush* GetMarqueeImage() const;

private:

	/** The style of the progress bar */
	const FProgressBarStyle* Style;

	/** The text displayed over the progress bar */
	TAttribute< TArray<float> > Percents;

	EBYGMultipartProgressBarFillType::Type BarFillType;

	/** Background image to use for the progress bar */
	const FSlateBrush* BackgroundImage;

	/** Foreground image to use for the progress bar */
	const FSlateBrush* FillImage;

	/** Image to use for marquee mode */
	const FSlateBrush* MarqueeImage;

	/** Fill Color and Opacity */
	TAttribute<TArray<FSlateColor>> FillColorAndOpacities;

	/** Border Padding */
	TAttribute<FVector2D> BorderPadding;

	/** Value to drive progress bar animation */
	float MarqueeOffset;

	float MarqueeScrollSpeed;

	bool bShowMarquee;

	/** Reference to the widgets current active timer */
	TWeakPtr<FActiveTimerHandle> ActiveTimerHandle;

	/** Rate at which the widget is currently ticked when slate sleep mode is active */
	float CurrentTickRate;

	/** The slowest that this widget can tick when in slate sleep mode */
	float MinimumTickRate;
};

