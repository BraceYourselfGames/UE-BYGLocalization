// Copyright 2017-2021 Brace Yourself Games. All Rights Reserved.

#include "SBYGMultipartProgressBar.h"
#include "Rendering/DrawElements.h"


void SBYGMultipartProgressBar::Construct( const FArguments& InArgs )
{
	check( InArgs._Style );

	MarqueeOffset = 0.0f;

	MarqueeScrollSpeed = InArgs._MarqueeScrollSpeed;
	bShowMarquee = false;

	Style = InArgs._Style;

	SetPercents( InArgs._Percents );
	BarFillType = InArgs._BarFillType;

	BackgroundImage = InArgs._BackgroundImage;
	FillImage = InArgs._FillImage;
	MarqueeImage = InArgs._MarqueeImage;

	FillColorAndOpacities = InArgs._FillColorAndOpacities;
	BorderPadding = InArgs._BorderPadding;

	CurrentTickRate = 0.0f;
	MinimumTickRate = InArgs._RefreshRate;

	ActiveTimerHandle = RegisterActiveTimer( CurrentTickRate, FWidgetActiveTimerDelegate::CreateSP( this, &SBYGMultipartProgressBar::ActiveTick ) );
}

void SBYGMultipartProgressBar::SetPercents( TAttribute< TArray<float> > InPercents )
{
	if ( !Percents.IdenticalTo( InPercents ) )
	{
		Percents = InPercents;
		Invalidate( EInvalidateWidget::LayoutAndVolatility );
	}
}

void SBYGMultipartProgressBar::SetStyle( const FProgressBarStyle* InStyle )
{
	Style = InStyle;

	if ( Style == nullptr )
	{
		FArguments Defaults;
		Style = Defaults._Style;
	}

	check( Style );

	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetBarFillType( EBYGMultipartProgressBarFillType::Type InBarFillType )
{
	BarFillType = InBarFillType;
	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetFillColorAndOpacities( TAttribute< TArray<FSlateColor> > InFillColorAndOpacities )
{
	FillColorAndOpacities = InFillColorAndOpacities;
	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetBorderPadding( TAttribute< FVector2D > InBorderPadding )
{
	BorderPadding = InBorderPadding;
	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetBackgroundImage( const FSlateBrush* InBackgroundImage )
{
	BackgroundImage = InBackgroundImage;
	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetFillImage( const FSlateBrush* InFillImage )
{
	FillImage = InFillImage;
	Invalidate( EInvalidateWidget::Layout );
}

void SBYGMultipartProgressBar::SetMarqueeImage( const FSlateBrush* InMarqueeImage )
{
	MarqueeImage = InMarqueeImage;
	Invalidate( EInvalidateWidget::Layout );
}

const FSlateBrush* SBYGMultipartProgressBar::GetBackgroundImage() const
{
	return BackgroundImage ? BackgroundImage : &Style->BackgroundImage;
}

const FSlateBrush* SBYGMultipartProgressBar::GetFillImage() const
{
	return FillImage ? FillImage : &Style->FillImage;
}

const FSlateBrush* SBYGMultipartProgressBar::GetMarqueeImage() const
{
	return MarqueeImage ? MarqueeImage : &Style->MarqueeImage;
}

// Returns false if the clipping zone area is zero in which case we should skip drawing
bool BYGMultipartPushTransformedClip( FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, FVector2D InsetPadding, FVector2D ProgressOrigin, FSlateRect Progress )
{
	const FSlateRenderTransform& Transform = AllottedGeometry.GetAccumulatedRenderTransform();

	const FVector2D MaxSize = AllottedGeometry.GetLocalSize() - ( InsetPadding * 2.0f );

	const FSlateClippingZone ClippingZone( Transform.TransformPoint( InsetPadding + ( ProgressOrigin - FVector2D( Progress.Left, Progress.Top ) ) * MaxSize ),
		Transform.TransformPoint( InsetPadding + FVector2D( ProgressOrigin.X + Progress.Right, ProgressOrigin.Y - Progress.Top ) * MaxSize ),
		Transform.TransformPoint( InsetPadding + FVector2D( ProgressOrigin.X - Progress.Left, ProgressOrigin.Y + Progress.Bottom ) * MaxSize ),
		Transform.TransformPoint( InsetPadding + ( ProgressOrigin + FVector2D( Progress.Right, Progress.Bottom ) ) * MaxSize ) );

	if ( ClippingZone.HasZeroArea() )
	{
		return false;
	}

	OutDrawElements.PushClip( ClippingZone );
	return true;
}

int32 SBYGMultipartProgressBar::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	// Used to track the layer ID we will return.
	int32 RetLayerId = LayerId;

	const ESlateDrawEffect DrawEffects = ESlateDrawEffect::None;

	const FSlateBrush* CurrentFillImage = GetFillImage();

	const FLinearColor ColorAndOpacitySRGB = InWidgetStyle.GetColorAndOpacityTint();

	TArray<float> ProgressFractions = Percents.Get();
	FVector2D BorderPaddingRef = BorderPadding.Get();

	const FSlateBrush* CurrentBackgroundImage = GetBackgroundImage();

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		RetLayerId++,
		AllottedGeometry.ToPaintGeometry(),
		CurrentBackgroundImage,
		DrawEffects,
		InWidgetStyle.GetColorAndOpacityTint() * CurrentBackgroundImage->GetTint( InWidgetStyle )
	);

	if ( bShowMarquee )
	{
		const FSlateBrush* CurrentMarqueeImage = GetMarqueeImage();

		// Draw Marquee
		const float MarqueeAnimOffset = CurrentMarqueeImage->ImageSize.X * MarqueeOffset;
		const float MarqueeImageSize = CurrentMarqueeImage->ImageSize.X;

		if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( 0, 0 ), FSlateRect( 0, 0, 1, 1 ) ) )
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(
					FVector2D( MarqueeAnimOffset - MarqueeImageSize, 0.0f ),
					FVector2D( AllottedGeometry.GetLocalSize().X + MarqueeImageSize, AllottedGeometry.GetLocalSize().Y ) ),
				CurrentMarqueeImage,
				DrawEffects,
				CurrentMarqueeImage->TintColor.GetSpecifiedColor() * ColorAndOpacitySRGB
			);

			OutDrawElements.PopClip();
		}
	}


	if ( ProgressFractions.Num() > 0 )
	{
		float RunningTotal = 0;
		
		for ( int32 i = 0; i < ProgressFractions.Num(); ++i )
		{
			FSlateColor FillColor = FSlateColor( FLinearColor::White );
			if ( FillColorAndOpacities.Get().Num() > i )
			{
				FillColor = FillColorAndOpacities.Get()[ i ];
			}
			const FLinearColor FillColorAndOpacitySRGB( InWidgetStyle.GetColorAndOpacityTint() * FillColor.GetColor( InWidgetStyle ) * CurrentFillImage->GetTint( InWidgetStyle ) );

			const float ClampedFraction = FMath::Clamp( ProgressFractions[ i ], 0.0f, 1.0f );
			switch ( BarFillType )
			{
			case EBYGMultipartProgressBarFillType::RightToLeft:
			{
				if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( 1, 0 ), FSlateRect( ClampedFraction, RunningTotal, 0, 1 ) ) )
				{
					// Draw Fill
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						RetLayerId++,
						AllottedGeometry.ToPaintGeometry(
							FVector2D::ZeroVector,
							FVector2D( AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y ) ),
						CurrentFillImage,
						DrawEffects,
						FillColorAndOpacitySRGB
					);

					OutDrawElements.PopClip();
				}
				break;
			}
			case EBYGMultipartProgressBarFillType::FillFromCenter:
			{
				float HalfFrac = ClampedFraction / 2.0f;
				if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( 0.5, 0.5 ), FSlateRect( HalfFrac, HalfFrac, HalfFrac, HalfFrac ) ) )
				{
					// Draw Fill
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						RetLayerId++,
						AllottedGeometry.ToPaintGeometry(
							FVector2D( ( AllottedGeometry.GetLocalSize().X * 0.5f ) - ( ( AllottedGeometry.GetLocalSize().X * ( ClampedFraction ) )*0.5 ), 0.0f ),
							FVector2D( AllottedGeometry.GetLocalSize().X * ( ClampedFraction ), AllottedGeometry.GetLocalSize().Y ) ),
						CurrentFillImage,
						DrawEffects,
						FillColorAndOpacitySRGB
					);

					OutDrawElements.PopClip();
				}
				break;
			}
			case EBYGMultipartProgressBarFillType::TopToBottom:
			{
				if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( 0, 0 ), FSlateRect( 0, 0, 1-RunningTotal, ClampedFraction ) ) )
				{
					// Draw Fill
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						RetLayerId++,
						AllottedGeometry.ToPaintGeometry(
							FVector2D::ZeroVector,
							FVector2D( AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y ) ),
						CurrentFillImage,
						DrawEffects,
						FillColorAndOpacitySRGB
					);

					OutDrawElements.PopClip();
				}
				break;
			}
			case EBYGMultipartProgressBarFillType::BottomToTop:
			{
				if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( 0, 1 ), FSlateRect( 0, ClampedFraction, 1, RunningTotal ) ) )
				{
					FSlateRect ClippedAllotedGeometry = FSlateRect( AllottedGeometry.AbsolutePosition, AllottedGeometry.AbsolutePosition + AllottedGeometry.GetLocalSize() * AllottedGeometry.Scale );
					ClippedAllotedGeometry.Top = ClippedAllotedGeometry.Bottom - ClippedAllotedGeometry.GetSize().Y * ClampedFraction;

					// Draw Fill
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						RetLayerId++,
						AllottedGeometry.ToPaintGeometry(
							FVector2D::ZeroVector,
							FVector2D( AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y ) ),
						CurrentFillImage,
						DrawEffects,
						FillColorAndOpacitySRGB
					);

					OutDrawElements.PopClip();
				}
				break;
			}
			case EBYGMultipartProgressBarFillType::LeftToRight:
			default:
			{
				if ( BYGMultipartPushTransformedClip( OutDrawElements, AllottedGeometry, BorderPaddingRef, FVector2D( RunningTotal, 0 ), FSlateRect( 0, 0, ClampedFraction, 1 ) ) )
				{
					// Draw Fill
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						RetLayerId++,
						AllottedGeometry.ToPaintGeometry(
							FVector2D::ZeroVector,
							FVector2D( AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y ) ),
						CurrentFillImage,
						DrawEffects,
						FillColorAndOpacitySRGB
					);

					OutDrawElements.PopClip();
				}
				break;
			}
			}

			RunningTotal += ClampedFraction;
		}
	}

	return RetLayerId - 1;
}

FVector2D SBYGMultipartProgressBar::ComputeDesiredSize( float ) const
{
	return GetMarqueeImage()->ImageSize;
}

bool SBYGMultipartProgressBar::ComputeVolatility() const
{
	return SLeafWidget::ComputeVolatility() || Percents.IsBound();
}

void SBYGMultipartProgressBar::SetActiveTimerTickRate( float TickRate )
{
	if ( CurrentTickRate != TickRate || !ActiveTimerHandle.IsValid() )
	{
		CurrentTickRate = TickRate;

		TSharedPtr<FActiveTimerHandle> SharedActiveTimerHandle = ActiveTimerHandle.Pin();
		if ( SharedActiveTimerHandle.IsValid() )
		{
			UnRegisterActiveTimer( SharedActiveTimerHandle.ToSharedRef() );
		}

		ActiveTimerHandle = RegisterActiveTimer( TickRate, FWidgetActiveTimerDelegate::CreateSP( this, &SBYGMultipartProgressBar::ActiveTick ) );
	}
}

EActiveTimerReturnType SBYGMultipartProgressBar::ActiveTick( double InCurrentTime, float InDeltaTime )
{
	MarqueeOffset = InCurrentTime - FMath::FloorToDouble( InCurrentTime ) * MarqueeScrollSpeed;

	TArray<float> PrecentFracton = Percents.Get();
	if ( PrecentFracton.Num() > 0 )
	{
		SetActiveTimerTickRate( MinimumTickRate );
	}
	else
	{
		SetActiveTimerTickRate( 0.0f );
	}

	return EActiveTimerReturnType::Continue;
}



void SBYGMultipartProgressBar::SetShowMarquee( bool bInShowMarquee )
{
	bShowMarquee = bInShowMarquee;
	Invalidate( EInvalidateWidget::Layout );
}


void SBYGMultipartProgressBar::SetMarqueeScrollSpeed( float InMarqueeScrollSpeed )
{
	MarqueeScrollSpeed = InMarqueeScrollSpeed;
}
