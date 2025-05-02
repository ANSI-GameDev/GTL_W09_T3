#pragma once
#include "Quat.h"
#include "Vector.h"

struct FTransform
{
protected:
    FVector Position;
    FQuat Rotation;
    FVector Scale;

public:
    static const FTransform Identity;

    FTransform()
        : Position(FVector(0, 0, 0))
        , Rotation(FQuat(0, 0, 0, 1))
        , Scale(FVector(1, 1, 1))
    {
    }

    FTransform(const FVector InPosition, const FRotator InRotation, const FVector InScale)
        : Position(InPosition)
        , Scale(InScale)
    {
        Rotation = InRotation.ToQuaternion();
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
		Rotation = InRotation.ToQuaternion();
	}

	FORCEINLINE void SetRotation(const FQuat& InQuat)
	{
		Rotation = InQuat;
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
	FQuat GetRotation() const
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
	// 쿼터니언을 회전 행렬로 변환한 후, 회전 행렬의 첫 번째 열(Forward 벡터)을 추출
	FVector GetForward() const;

	// 객체의 오른쪽 벡터를 반환하는 함수
	// 글로벌 Up 벡터 (0,0,1)와 전방 벡터의 외적을 계산하여 정규화
	FVector GetRight() const;

	// 객체의 Up 벡터를 반환하는 함수
	// 전방 벡터와 오른쪽 벡터의 외적을 계산하여 정규화
	FVector GetUp() const;

	// 로컬 좌표계의 오른쪽 벡터를 반환하는 함수 (회전 행렬의 두 번째 열 사용)
	FVector GetLocalRight() const;

	// 로컬 좌표계의 Up 벡터를 반환하는 함수 (회전 행렬의 세 번째 열 사용)
	FVector GetLocalUp() const;

	// 객체를 이동시키는 함수 (현재 위치에 InTranslation 값을 더함)
	void Translate(const FVector& InTranslation);

	// Euler 각도(도 단위)를 사용하여 객체를 회전시키는 함수
	// InRotation의 X, Y, Z 성분에 따라 Roll, Pitch, Yaw 회전을 차례로 적용
	void Rotate(const FVector& InRotation);

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
    
	// 두 변환(Transform)을 합성하는 함수 (곱셈 순서에 주의)
	// Result = A 변환을 적용한 후 B 변환을 적용.
	// 회전: Result.Rotation = A.Rotation * B.Rotation
	// Translation: Result.Position = B.Rotation.RotateVector(B.Scale * A.Translation) + B.Translation
	// Scale: Result.Scale = A.Scale * B.Scale (성분별 곱셈)
	static FTransform MultiPly(const FTransform& A, const FTransform& B);

	// 4D 벡터를 이 변환으로 변환하는 함수.
	// 입력 벡터의 W 값이 1이면 점(위치), 0이면 방향으로 취급한다.
	// 변환 공식: V' = Q.Rotate(S * V) + T, 단 x,y,z에만 적용되고, Translation은 W에 따라 적용됨.
	FVector4 TransformVector4(const FVector4& Vector) const;

	// 점(위치) 변환 함수: P' = Q.Rotate(S * P) + T
	FVector TransformPosition(const FVector& Vector) const;

	// 스케일을 적용하지 않고 점(위치) 변환: P' = Q.Rotate(P) + T
	FVector TransformPositionNoScale(const FVector& Vector) const;

	// 벡터(방향) 변환 함수: V' = Q.Rotate(S * V)
	// Translation은 방향 벡터에 적용되지 않음.
	FVector TransformVector(const FVector& Vector) const;

	// 스케일 없이 벡터(방향) 변환: V' = Q.Rotate(V)
	FVector TransformVectorNoScale(const FVector& Vector) const;

	// 회전 없이 스케일만 적용한 벡터 변환 함수.
	FVector TransformVectorNoRotation(const FVector& Vector) const;

	// 점(위치)에 대해 역변환을 수행하는 함수.
	// 역변환 공식: P = (Q^-1.Rotate(P' - T)) / Scale  (각 축별 나눔)
	FVector InverseTransformPosition(const FVector& Vector) const;

	// 스케일 없이 점(위치)에 대한 역변환: P = Q^-1.Rotate(P' - T)
	FVector InverseTransformPositionNoScale(const FVector& Vector) const;

	// 벡터(방향)에 대한 역변환 함수: V = (Q^-1.Rotate(V)) / Scale
	FVector InverseTransformVector(const FVector& Vector) const;

	// 스케일 없이 벡터(방향)에 대한 역변환: V = Q^-1.Rotate(V)
	FVector InverseTransformVectorNoScale(const FVector& Vector) const;

	// 입력 쿼터니언(InRotation)을 현재 회전(Rotation)과 합성하여 회전 변환을 적용하는 함수.
	// 결과 회전 = 현재 Rotation * InRotation.
	FQuat TransformRotation(const FQuat& InRotation) const;

	// 입력 쿼터니언(InRotation)에 대해 현재 회전의 역을 적용하여 역회전 변환을 수행하는 함수.
	// 결과 = 현재 Rotation^-1 * InRotation.
	FQuat InverseTransformRotation(const FQuat& InRotation) const;

	/**
	 * @brief 현재 Transform(this)이 주어진 Other Transform에 대해 Relative Transform을 계산합니다.
	 *
	 * 두 변환 A (this)와 B (Other)가 있을 때, 상대 변환 T_rel을 아래와 같이 계산합니다.
	 *
	 *   T_rel = B⁻¹ * A
	 *
	 * 각 구성 요소는 다음과 같이 계산됩니다:
	 *  - 상대 스케일: S(A) / S(B) (컴포넌트별 나눗셈)
	 *  - 상대 회전: Q(B)⁻¹ * Q(A)
	 *  - 상대 Translation: (1 / S(B)) * [ Q(B)⁻¹ * (T(A) - T(B)) ]
	 *
	 * @param Other 기준 Transform(B). 이 Transform에 대해 현재 Transform의 상대값을 구합니다.
	 * @return FTransform Other에 대한 현재 Transform의 Relative Transform.
	 */
	FTransform GetRelativeTransform(const FTransform& Other) const;
    
	/**
	* @brief 반대(Reverse) 상대 변환을 계산합니다.
	*
	* 두 변환 A (this)와 B (Other)가 있을 때, 아래와 같이 반대 상대 변환을 정의합니다.
	*
	*   상대 스케일 = S(B) / S(A)
	*   상대 회전 = Q(B) * Q(A)⁻¹
	*   상대 Translation = T(B) - [ 상대 스케일 * (RelativeRotation.RotateVector(T(A)) ) ]
	*
	* 이 함수는 A의 역변환을 적용한 후 B가 나오도록 하는 Transform을 계산합니다.
	*
	* @param Other Transform(B). 이 Transform에 대해 Inverse Relative Transform을 구합니다.
	* @return FTransform 계산된 Inverse Relative Transform.
	*/
	FTransform GetRelativeTransformReverse(const FTransform& Other) const;

	/**
	* @brief 부모 Transform(ParentTransform)에 대해 현재 Transform의(this)을 로컬(상대) Transform으로 업데이트합니다.
	*
	* 이 함수는 현재 Transform을 부모 Transform에 대한 상대(로컬) Transform으로 변경합니다.
	* 계산 과정은 다음과 같습니다:
	*  - Scale: 현재 Scale을 부모의 Scale로 나눔 (컴포넌트별)
	*  - Translation:
	*       (1) 부모의 위치를 빼서 위치 차이를 구함
	*       (2) 부모의 회전의 역수를 적용하여 부모 좌표계로 변환
	*       (3) 부모의 Scale로 나누어 보정
	*  - Rotation: 부모의 회전의 역수 * 현재 회전
	*
	* @param ParentTransform 부모 Transform. 이 Transform에 대해 현재 Transform을 Local Transform으로 변경합니다.
	*/
	static FTransform SetToRelativeTransform(const FTransform& myTransform, const FTransform& ParentTransform);

	/**
	* @brief 행렬 연산을 사용하여 스케일을 포함한 Relative Transform을 계산합니다.
	*
	* Base 변환과 Relative 변환을 각각 행렬로 변환한 후,
	* 다음 단계를 거쳐 Relative Transform을 계산합니다:
	*   1. Base와 Relative Transform을 행렬로 변환 (스케일 포함)
	*   2. 원하는 상대 스케일 DesiredScale을 계산: S(Base) / S(Relative) (컴포넌트별)
	*   3. Relative 행렬 BM의 역행렬을 구하고, Base 행렬 AM과 곱하여 상대 행렬 M을 구함.
	*   4. 구해진 행렬 M과 DesiredScale을 이용하여 최종 Relative Transform을 구성.
	*
	* @param Base Base Transform(A).
	* @param Relative Relative Transform(B).
	* @return FTransform 계산된 Relative Transform.
	*/
	FTransform GetRelativeTransformUsingMatrixWithScale(const FTransform* Base, const FTransform* Relative) const;

	// ConstructTransformFromMatrixWithDesiredScale 함수
	// InMatrix : 변환 행렬 (스케일이 포함된 회전+이동 행렬)
	// DesiredScale : 원하는 스케일 (컴포넌트별 값)
	// OutTransform : 복원된 변환(Translation, Rotation, Scale)을 저장할 대상
	FTransform ConstructTransformFromMatrixWithDesiredScale(const FMatrix& InMatrix, const FVector& DesiredScale) const;

	FTransform Inverse() const;

	void ToMatrixInternal(FVector& OutDiagonals, FVector& OutAdds, FVector& OutSubtracts) const;

	FMatrix ToMatrixWithScale() const;
};
