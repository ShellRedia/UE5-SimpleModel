// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/ImageUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/Material.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

#if WITH_OPENCV
#include "PreOpenCVHeaders.h"
#include "OpenCVHelper.h"
#include "PostOpenCVHeaders.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#endif
// #include "opencv2/imgcodecs.hpp"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"

#include "cmath"

#include "SceneShot.generated.h"

UCLASS()
class SIMPLEMODEL_API ASceneShot : public APawn
{
	GENERATED_BODY()
	
public:	
	
	ASceneShot(); // Sets default values for this actor's properties

	virtual void Tick(float DeltaTime) override; 	// Called every frame

	UFUNCTION(BlueprintCallable)
	FVector2D ProjectWorldToSceneCapture(const FVector& WorldPosition);

	UFUNCTION(BlueprintCallable)
	void ScreenshotToImage(const FString& InImagePath, const FVector2D& InRangeSize);

	UFUNCTION(BlueprintCallable)
	void ColorToImage(const FString& InImagePath, TArray<FColor> InColor, int32 InWidth, int32 InHeight);

	UFUNCTION()
	TArray<FColor> ScreenshotToColor();

	UFUNCTION()
	UTexture2D* ColorToTexture(TArray<FColor> InColor, int32 InWidth, int32 InHeight);

	UFUNCTION(BlueprintPure)
	UTexture2D* GetProcessedTexture();

	cv::Mat ColorToCV2Mat(TArray<FColor> InColor, int32 Width, int32 Height);
	TArray<FColor> CV2MatToColor(cv::Mat rgbaImage);

	void DrawLinesConnectKeypoints(cv::Mat& rgbaImage);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* CaptureComponent2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TArray<FVector> KeypointsLoc;
};
