#pragma once
#include "Quat.h"
#include "Vector.h"

struct FTransform
{
protected:
    FVector Position;
    FRotator Rotation;
    FVector Scale;

public:
    static const FTransform Identity;

    FTransform()
        : Position(FVector(0, 0, 0))
        , Rotation(FQuat(0, 0, 0, 1))
        , Scale(FVector(1, 1, 1))
    {
    }

    FTransform(const FVector InPosition, const FRotator& InRotation, const FVector InScale)
        : Position(InPosition)
        , Scale(InScale)
    {
        Rotation = InRotation;
    }

    FTransform(const FVector InPosition, const FQuat InQuat, const FVector InScale)
        : Position(InPosition)
        , Rotation(InQuat)
        , Scale(InScale)
    {
    }

    FORCEINLINE bool Equal(const FTransform& Other) const
	{
		return Position == Other.Position && Rotation == Other.Rotation && Scale == Other.Scale;
	}
	        
	// 뷰 행렬을 구하는 함수 (왼손 좌표계, LookAtLH 방식)
	// 카메라의 위치(Position), 카메라가 바라보는 위치(Position + GetForward()), 그리고 카메라의 Up 벡터를 사용함.
	FMatrix GetViewMatrix() const;

	// 객체의 위치를 설정하는 함수 (벡터를 인자로 받음)
	FORCEINLINE void SetPosition(const FVector& InPosition)
	{
		Position = InPosition;
	}

	// 객체의 위치를 설정하는 함수 (x, y, z 값을 각각 인자로 받음)
	FORCEINLINE void SetPosition(float x, float y, float z)
	{
		Position = { x, y, z };
	}

	// 객체의 회전을 설정하는 함수 (Euler 각도 벡터를 인자로 받아 쿼터니언으로 변환)
	FORCEINLINE void SetRotation(const FRotator& InRotation)
	{
		Rotation = InRotation;
	}

	FORCEINLINE void SetRotation(const FQuat& InQuat)
	{
		Rotation = InQuat.Rotator();
	}

	// 객체의 회전을 설정하는 함수 (x, y, z Euler 각도를 각각 인자로 받음)
	inline void SetRotation(const float x, const float y, const float z)
	{
		SetRotation(FRotator(x, y, z));
	}

	// 객체의 스케일을 설정하는 함수 (벡터로 설정)
	FORCEINLINE void SetScale(const FVector InScale)
	{
		Scale = InScale;
	}

	// 객체의 스케일에 값을 더하는 함수 (각 축별로 증가)
	FORCEINLINE void AddScale(const FVector InScale)
	{
		Scale.X += InScale.X;
		Scale.Y += InScale.Y;
		Scale.Z += InScale.Z;
	}

	// 객체의 스케일을 설정하는 함수 (x, y, z 값을 각각 인자로 받음)
	FORCEINLINE void SetScale(float x, float y, float z)
	{
		Scale = { x, y, z };
	}

	// 객체의 현재 위치를 반환하는 함수
	FVector GetPosition() const
	{
		return Position;
	}

	// 객체의 현재 회전을 반환하는 함수 (쿼터니언으로 반환)
	FRotator GetRotation() const
	{
		return Rotation;
	}

	// 객체의 현재 스케일을 반환하는 함수
	FVector GetScale() const
	{
		return Scale;
	}

	// 스케일 행렬을 반환하는 함수 (객체의 스케일 값으로 구성된 행렬)
	FMatrix GetScaleMatrix() const;

	// 전체 변환 행렬을 반환하는 함수
	// 구성 순서: Scale 행렬 * Rotation 행렬 * Translation 행렬
	FMatrix GetMatrix() const;

	// 스케일을 제외한 로컬 변환 행렬(회전 및 이동만 적용된 행렬)을 반환하는 함수
	FMatrix GetLocalMatrixWithOutScale() const;

	// 객체가 바라보는 전방 벡터를 반환하는 함수
	// 회전 행렬로 변환한 후, 회전 행렬의 첫 번째 열(Forward 벡터)을 추출
	FVector GetForward() const;

	// 객체의 오른쪽 벡터를 반환하는 함수
	FVector GetRight() const;

	// 객체의 Up 벡터를 반환하는 함수
	FVector GetUp() const;

	// 객체를 이동시키는 함수 (현재 위치에 InTranslation 값을 더함)
	void Translate(const FVector& InTranslation);

	// Euler 각도(도 단위)를 사용하여 객체를 회전시키는 함수
	// InRotation의 X, Y, Z 성분에 따라 Roll, Pitch, Yaw 회전을 차례로 적용
	void Rotate(const FRotator& InRotation);

	// Yaw 회전 (Z축 기준 회전)을 적용하는 함수
	void RotateYaw(float Angle);

	// Pitch 회전 (Y축 기준 회전)을 적용하는 함수
	void RotatePitch(float Angle);

	// Roll 회전 (X축 기준 회전)을 적용하는 함수
	void RotateRoll(float Angle);

	// 객체를 로컬 좌표계에서 이동시키는 함수
	// 로컬 변환 행렬을 사용해 InTranslation 벡터를 월드 좌표로 변환한 후 현재 위치에 더함
	void MoveLocal(const FVector& InTranslation);

	// InMatrix 행렬에서 스케일, 회전(쿼터니언), Translation을 추출하여 현재 변환을 설정하는 함수
	void SetFromMatrix(const FMatrix& InMatrix);

	// 4D 벡터를 이 변환으로 변환하는 함수.
	// 입력 벡터의 W 값이 1이면 점(위치), 0이면 방향으로 취급한다.
	// 변환 공식: V' = Q.Rotate(S * V) + T, 단 x,y,z에만 적용되고, Translation은 W에 따라 적용됨.
	FVector4 TransformVector4(const FVector4& Vector) const;

	// 점(위치) 변환 함수: P' = Q.Rotate(S * P) + T
	FVector TransformPosition(const FVector& Vector) const;

	// 벡터(방향) 변환 함수: V' = Q.Rotate(S * V)
	// Translation은 방향 벡터에 적용되지 않음.
	FVector TransformVector(const FVector& Vector) const;
    
	FTransform Inverse() const;

    FTransform operator*(const FTransform& Other) const;
};
