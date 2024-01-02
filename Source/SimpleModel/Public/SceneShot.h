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
#endif
// #include "opencv2/imgcodecs.hpp"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
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
	void ColorToImage(const FString& InImagePath, TArray<FColor>InColor, int32 InWidth, int32 InHeight);

	UFUNCTION()
	TArray<FColor> ScreenshotToColor();

	UFUNCTION()
	UTexture2D* ColorToTexture(TArray<FColor>InColor, int32 InWidth, int32 InHeight);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (AllowPrivateAccess="true"))
	USceneCaptureComponent2D* CaptureComponent2D;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;
};
