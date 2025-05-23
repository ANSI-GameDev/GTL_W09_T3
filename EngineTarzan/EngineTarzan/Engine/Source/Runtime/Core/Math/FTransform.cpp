#include "FTransform.h"

#include "JungleMath.h"

FMatrix FTransform::GetViewMatrix() const
{
    return JungleMath::CreateViewMatrix(Position, Position + GetForward(), FVector::UpVector);
}

FMatrix FTransform::GetScaleMatrix() const
{
    return FMatrix::GetScaleMatrix(Scale);
}

FMatrix FTransform::GetMatrix() const
{
    return FMatrix::GetScaleMatrix(Scale)
        * FMatrix::GetRotationMatrix(Rotation)
        * FMatrix::GetTranslationMatrix(Position);
}

FMatrix FTransform::GetLocalMatrixWithOutScale() const
{
    return FMatrix::GetRotationMatrix(Rotation)
        * FMatrix::GetTranslationMatrix(Position);
}

FVector FTransform::GetForward() const
{
    const FMatrix RotationMatrix = FMatrix::GetRotationMatrix(Rotation);
    const FVector Forward = FVector(
        RotationMatrix.M[0][0],
        RotationMatrix.M[1][0],
        RotationMatrix.M[2][0]
    );
    return Forward.GetSafeNormal();
}

FVector FTransform::GetRight() const
{
    const FMatrix RotationMatrix = FMatrix::GetRotationMatrix(Rotation);
    const FVector Right = FVector(
        RotationMatrix.M[0][1],
        RotationMatrix.M[1][1],
        RotationMatrix.M[2][1]
    );
    return Right.GetSafeNormal();
}

FVector FTransform::GetUp() const
{
    const FMatrix RotationMatrix = FMatrix::GetRotationMatrix(Rotation);
    const FVector up = FVector(
        RotationMatrix.M[0][2],
        RotationMatrix.M[1][2],
        RotationMatrix.M[2][2]
    );
    return up.GetSafeNormal();
}

void FTransform::Translate(const FVector& InTranslation)
{
    Position += InTranslation;
}

void FTransform::Rotate(const FRotator& InRotation)
{
    RotateRoll(InRotation.Roll);
    RotatePitch(InRotation.Pitch);
    RotateYaw(InRotation.Yaw);
}

void FTransform::RotateYaw(const float AngleDegrees)
{
    // Z축(Up) 기준 Yaw 회전
    const float HalfRad = FMath::DegreesToRadians(AngleDegrees) * 0.5f;
    const float SinH = FMath::Sin(HalfRad);
    const float CosH = FMath::Cos(HalfRad);
    FQuat DeltaQuat(FVector(0, 0, 1) * SinH, CosH);
    DeltaQuat.Normalize();
    
    Rotation = ((DeltaQuat) * Rotation.ToQuaternion()).Rotator();
}

void FTransform::RotatePitch(const float AngleDegrees)
{
    // Y축(Right) 기준 Pitch 회전
    const float HalfRad = FMath::DegreesToRadians(AngleDegrees) * 0.5f;
    const float SinH = FMath::Sin(HalfRad);
    const float CosH = FMath::Cos(HalfRad);
    FQuat DeltaQuat(FVector(0, 1, 0) * SinH, CosH);
    DeltaQuat.Normalize();
    
    Rotation = ((DeltaQuat) * Rotation.ToQuaternion()).Rotator();
}

void FTransform::RotateRoll(const float AngleDegrees)
{
    // X축(Forward) 기준 Roll 회전
    const float HalfRad = FMath::DegreesToRadians(AngleDegrees) * 0.5f;
    const float SinH = FMath::Sin(HalfRad);
    const float CosH = FMath::Cos(HalfRad);
    FQuat DeltaQuat(FVector(1, 0, 0) * SinH, CosH);
    DeltaQuat.Normalize();
    
    Rotation = ((DeltaQuat) * Rotation.ToQuaternion()).Rotator();
}


void FTransform::MoveLocal(const FVector& InTranslation)
{
    const FMatrix transfromMatrix = GetMatrix();
    const FVector worldDelta = FMatrix::TransformVector(InTranslation, transfromMatrix);
    Position += worldDelta;
}

void FTransform::SetFromMatrix(const FMatrix& InMatrix)
{
    FMatrix matrix = InMatrix;

    // 행렬에서 스케일 추출
    FVector scale = matrix.ExtractScale();
    Scale = scale;

    // 행렬식이 음수면 스케일 부호를 보정
    if (matrix.Determinant() < 0)
    {
        scale *= -1;
        Scale = Scale * FVector(-1.f, 1.f, 1.f);
    }

    // 회전 부분을 쿼터니언으로 변환
    const FQuat quat = FQuat(matrix);
    Rotation = quat.Rotator();

    // Translation(이동) 추출
    const FVector translation = matrix.GetTranslation();
    Position = translation;
}

FVector4 FTransform::TransformVector4(const FVector4& Vector) const
{
    // 입력 4D 벡터에서 x,y,z를 추출
    const FVector vec3 = FVector(Vector.X, Vector.Y, Vector.Z);

    // 스케일 적용
    const FVector scaledVec3 = vec3 * Scale;

    // 회전 적용
    const FVector rotatedVec3 = Rotation.RotateVector(scaledVec3);

    // Translation은 W 값에 따라 적용 (W가 1이면 적용, 0이면 무시)
    const FVector translatedVec3 = rotatedVec3 + (Position * Vector.W);

    FVector4 result;
    result.X = translatedVec3.X;
    result.Y = translatedVec3.Y;
    result.Z = translatedVec3.Z;
    result.W = Vector.W;  // W값은 그대로 유지
    return result;
}

FVector FTransform::TransformPosition(const FVector& Vector) const
{
    const FVector Scaled = Vector * Scale;
    const FVector Rotated = Rotation.RotateVector(Scaled);
    return Rotated + Position;
}


FVector FTransform::TransformVector(const FVector& Vector) const
{
    const FVector scaledVec3 = Vector * Scale;
    const FVector rotatedVec3 = Rotation.RotateVector(scaledVec3);
    return rotatedVec3;
}

FTransform FTransform::Inverse() const
{
    const FVector4 InverseScale = FVector4(1.f / Scale.X, 1.f / Scale.Y, 1.f / Scale.Z, 0.f);
    const FRotator InverseRotation = Rotation.GetInverse();

    const FVector ScaledTranslation = FVector(InverseScale.X * Position.X, InverseScale.Y * Position.Y, InverseScale.Z * Position.Z);
    const FVector RotatedTranslation = InverseRotation.RotateVector(-ScaledTranslation);

    return FTransform(RotatedTranslation, InverseRotation, FVector(InverseScale.X, InverseScale.Y, InverseScale.Z));
}

FTransform FTransform::operator*(const FTransform& Other) const
{
    //FMatrix ScaleMat = GetScaleMatrix();
    //FMatrix RotationMat = GetRotation().ToMatrix();
    //FMatrix TranslationMat = FMatrix::GetTranslationMatrix(Position);
    //
    //FMatrix OtherScaleMat = Other.GetScaleMatrix();
    //FMatrix OtherRotMat = Other.GetRotation().ToMatrix();
    //FMatrix OtherTranslationMat = FMatrix::GetTranslationMatrix(Other.Position);
    //
    //FMatrix RTMat = RotationMat * TranslationMat;
    //FMatrix OtherRTMat = OtherRotMat * OtherTranslationMat;
    //
    //ScaleMat = ScaleMat * OtherScaleMat;
    //
    //FMatrix ResultRTMat = OtherRTMat * RTMat;
    //FMatrix Result = ScaleMat * ResultRTMat;
    //
    //return FTransform(Result.GetTranslation(), Result.GetMatrixWithoutScale().ToQuat().GetNormalized(), Result.GetScaleVector());

    FVector ResultScale = Scale * Other.GetScale();
    FRotator ResultRotation = Rotation * Other.GetRotation();
    FVector ScaledPosition = Rotation.ToMatrix().TransformPosition(Other.GetPosition() * Scale);
    FVector ResultPosition = Position + ScaledPosition;

    return FTransform{ ResultPosition, ResultRotation, ResultScale };
}
