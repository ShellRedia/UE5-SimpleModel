// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneShot.h"

// Sets default values
ASceneShot::ASceneShot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NewSceneCaptureComponent2D"));
	CaptureComponent2D->SetupAttachment(RootComponent);
	CaptureComponent2D->bOverride_CustomNearClippingPlane = true;
	CaptureComponent2D->CustomNearClippingPlane = 0.01f;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("NewCameraComponent2D"));
	CameraComponent->SetupAttachment(CaptureComponent2D);
	CameraComponent->AddRelativeLocation(FVector(-20, 0, 0));
}

// Called when the game starts or when spawned
void ASceneShot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASceneShot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector2D ASceneShot::ProjectWorldToSceneCapture(const FVector& WorldPosition){
	FVector2D ScreenPosition = FVector2d::Zero();
	if (CaptureComponent2D){
		FMatrix OutViewMatrix;
		FMatrix ProjectionMatrix;
		FMatrix ViewProjectionMatrix;
		TOptional<FMatrix> CustomProjectionMatrix;
		if (CaptureComponent2D->bUseCustomProjectionMatrix) CustomProjectionMatrix = CaptureComponent2D->CustomProjectionMatrix;
		FMinimalViewInfo ViewInfo;
		CaptureComponent2D->GetCameraView(0.0f, ViewInfo);

		UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(ViewInfo, CustomProjectionMatrix, OutViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
		FIntPoint TargetSize = FIntPoint(
			CaptureComponent2D->TextureTarget->SizeX,
			CaptureComponent2D->TextureTarget->SizeY);
		FIntRect(FIntPoint(0, 0), TargetSize);

		bool bFlag = FSceneView::ProjectWorldToScreen(WorldPosition, FIntRect(FIntPoint(0, 0), TargetSize), ViewProjectionMatrix, ScreenPosition);
		if (bFlag) return ScreenPosition;
	}
	return ScreenPosition;
}

TArray<FColor> ASceneShot::ScreenshotToColor() {
	TArray<FColor> OutColor;
	FTextureRenderTargetResource* TextureRenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
	TextureRenderTargetResource->ReadPixels(OutColor);
	return OutColor;
}

void ASceneShot::ScreenshotToImage(const FString& InImagePath, const FVector2D& InRangeSize) {
	if (CaptureComponent2D && CaptureComponent2D->TextureTarget)
	{
		CaptureComponent2D->TextureTarget->InitAutoFormat(InRangeSize.X, InRangeSize.Y);
		CaptureComponent2D->UpdateContent(); //画面刷新

		//获取RenderTarget贴图的资源
		auto Lab = [=, this] {
			FTextureRenderTargetResource* TextureRenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
			int32 Width = CaptureComponent2D->TextureTarget->SizeX;
			int32 Height = CaptureComponent2D->TextureTarget->SizeY;
			TArray<FColor> OutData;
			TextureRenderTargetResource->ReadPixels(OutData);
			ColorToImage(InImagePath, OutData, Width, Height);
		};
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, Lab, 0.1f, false, 0);
	}
}

void ASceneShot::ColorToImage(const FString& InImagePath, TArray<FColor>InColor, int32 InWidth, int32 InHeight) {
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	FString Ex = FPaths::GetExtension(InImagePath);
	if (Ex.Equals(TEXT("jpg"), ESearchCase::IgnoreCase) || Ex.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase)) {
		TSharedPtr<IImageWrapper>ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		if (ImageWrapper->SetRaw(InColor.GetData(), InColor.GetAllocatedSize(), InWidth, InHeight, ERGBFormat::BGRA, 8)) {
			FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(100), *InImagePath);
		}
	}
	else {
		TArray<uint8> OutPNG;
		for (FColor& color : InColor) color.A = 255;
		FImageUtils::CompressImageArray(InWidth, InHeight, InColor, OutPNG);
		FFileHelper::SaveArrayToFile(OutPNG, *InImagePath);
	}
}

UTexture2D* ASceneShot::ColorToTexture(TArray<FColor>InColor, int32 InWidth, int32 InHeight) {
	UTexture2D* Texture = UTexture2D::CreateTransient(InWidth, InHeight, PF_B8G8R8A8);

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, InColor.GetData(), InColor.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();

	Texture->UpdateResource();
	return Texture;
}