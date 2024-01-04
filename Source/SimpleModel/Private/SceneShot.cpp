// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneShot.h"

// Sets default values
ASceneShot::ASceneShot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));


	CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	CaptureComponent2D->SetupAttachment(RootComponent);
	CaptureComponent2D->bOverride_CustomNearClippingPlane = true;
	CaptureComponent2D->CustomNearClippingPlane = 0.01f;
	CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
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

void ASceneShot::ScreenshotToImage(const FString& InImagePath, const FVector2D& InRangeSize) {
	if (CaptureComponent2D && CaptureComponent2D->TextureTarget){
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

void ASceneShot::ColorToImage(const FString& InImagePath, TArray<FColor> InColor, int32 InWidth, int32 InHeight) {
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

TArray<FColor> ASceneShot::ScreenshotToColor() {
	TArray<FColor> OutColor;
	//CaptureComponent2D->UpdateContent();
	CaptureComponent2D->UpdateContent();
	FTextureRenderTargetResource* TextureRenderTargetResource = CaptureComponent2D->TextureTarget->GameThread_GetRenderTargetResource();
	TextureRenderTargetResource->ReadPixels(OutColor);
	return OutColor;
}

UTexture2D* ASceneShot::ColorToTexture(TArray<FColor> InColor, int32 InWidth, int32 InHeight) {
	UTexture2D* Texture = UTexture2D::CreateTransient(InWidth, InHeight, PF_B8G8R8A8);
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, InColor.GetData(), InColor.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	Texture->UpdateResource();
	return Texture;
}

cv::Mat ASceneShot::ColorToCV2Mat(TArray<FColor> InColor, int32 Width, int32 Height) {
	cv::Mat rgbaImage = cv::Mat::zeros(Height, Width, CV_8UC4);
	int color_idx = 0;

	for (int i = 0; i < rgbaImage.rows; i++) {
		for (int j = 0; j < rgbaImage.cols; j++) {
			cv::Vec4b& pixel_mat = rgbaImage.at<cv::Vec4b>(i, j);
			pixel_mat[0] = (uint8)std::min(255, 1 * InColor[color_idx].B);
			pixel_mat[1] = (uint8)std::min(255, 1 * InColor[color_idx].G);
			pixel_mat[2] = (uint8)std::min(255, 1 * InColor[color_idx].R);
			pixel_mat[3] = (uint8)255;  // Alpha
			color_idx++;
		}
	}
	return rgbaImage;
}

TArray<FColor> ASceneShot::CV2MatToColor(cv::Mat rgbaImage){
	TArray<FColor> OutColor;
	for (int i = 0; i < rgbaImage.rows; ++i) {
		for (int j = 0; j < rgbaImage.cols; ++j) {
			cv::Vec4b pixel_mat = rgbaImage.at<cv::Vec4b>(i, j);
			FColor pixel_color = FColor(pixel_mat[2], pixel_mat[1], pixel_mat[0], pixel_mat[3]);
			OutColor.Add(pixel_color);
		}
	}
	return OutColor;
}

UTexture2D* ASceneShot::GetProcessedTexture() {
	TArray<FColor> ScreenColor = ScreenshotToColor();

	int32 Width = CaptureComponent2D->TextureTarget->SizeX;
	int32 Height = CaptureComponent2D->TextureTarget->SizeY;

	cv::Mat rgbaImage = ColorToCV2Mat(ScreenColor, Width, Height);
	DrawLinesConnectKeypoints(rgbaImage);
	TArray<FColor> ProcessedColor = CV2MatToColor(rgbaImage);

	return ColorToTexture(ProcessedColor, Width, Height);
}

void ASceneShot::DrawLinesConnectKeypoints(cv::Mat &rgbaImage) {
	cv::Scalar colorR(0, 0, 255, 255), colorB(255, 0, 0, 255), colorY(0, 255, 255, 255);
	int thickness = 4;
	TArray<int> ConnectFrom_R = { 1, 1, 1, 2, 3, 4, 4, 5, 5, 6, 7 }; 
	TArray<int> ConnectTo_R = { 2, 8, 9, 3, 4, 5, 6, 6, 7, 8, 9 };

	TArray<int> ConnectFrom_L = { 10, 10, 10, 11, 12, 13, 13, 14, 14, 15, 16 };
	TArray<int> ConnectTo_L = { 11, 17, 18, 12, 13, 14, 15, 15, 16, 17, 18 };

	TArray<int> ConnectFrom_M = { 5, 9 };
	TArray<int> ConnectTo_M = { 14, 18 };

	if (KeypointsLoc.Num()) {
		for (int i = 0; i < ConnectFrom_R.Num(); i++) {
			int idx_a = ConnectFrom_R[i], idx_b = ConnectTo_R[i];
			FVector2D point_a = ProjectWorldToSceneCapture(KeypointsLoc[idx_a-1]);
			FVector2D point_b = ProjectWorldToSceneCapture(KeypointsLoc[idx_b-1]);
			cv::Point start_point(point_a.X, point_a.Y);
			cv::Point end_point(point_b.X, point_b.Y);
			cv::line(rgbaImage, start_point, end_point, colorR, thickness);
		}
		for (int i = 0; i < ConnectFrom_L.Num(); i++) {
			int idx_a = ConnectFrom_L[i], idx_b = ConnectTo_L[i];
			FVector2D point_a = ProjectWorldToSceneCapture(KeypointsLoc[idx_a - 1]);
			FVector2D point_b = ProjectWorldToSceneCapture(KeypointsLoc[idx_b - 1]);
			cv::Point start_point(point_a.X, point_a.Y);
			cv::Point end_point(point_b.X, point_b.Y);
			cv::line(rgbaImage, start_point, end_point, colorY, thickness);
		}
		for (int i = 0; i < ConnectFrom_M.Num(); i++) {
			int idx_a = ConnectFrom_M[i], idx_b = ConnectTo_M[i];
			FVector2D point_a = ProjectWorldToSceneCapture(KeypointsLoc[idx_a - 1]);
			FVector2D point_b = ProjectWorldToSceneCapture(KeypointsLoc[idx_b - 1]);
			cv::Point start_point(point_a.X, point_a.Y);
			cv::Point end_point(point_b.X, point_b.Y);
			cv::line(rgbaImage, start_point, end_point, colorB, thickness);
		}
	}
}