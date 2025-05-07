#pragma once
#include "Rotator.h"
#include "Serialization/Archive.h"
struct FVector;
struct FMatrix;
struct FVector4;
// 쿼터니언
struct FQuat
{
    float X, Y, Z, W;
    // 기본 생성자
    explicit FQuat()
        : W(1.0f), X(0.0f), Y(0.0f), Z(0.0f)
    {
    }
    // FQuat 생성자 추가: 회전 축과 각도를 받아서 FQuat 생성
    explicit FQuat(const FVector& Axis, float Angle);
    // W, X, Y, Z 값으로 초기화
    explicit FQuat(const float InX, const float InY, const float InZ, const float InW)
        : W(InW), X(InX), Y(InY), Z(InZ)
    {
    }
    explicit FQuat(const FRotator& InRotator)
    {
        *this = InRotator.ToQuaternion();
    }
    explicit FQuat(const FMatrix& InMatrix);
    //explicit FQuat(const FVector& Rotation);
    // 쿼터니언의 곱셈 연산 (회전 결합)
    FQuat operator*(const FQuat& Other) const;
    bool operator==(const FQuat& Other) const;
    bool operator!=(const FQuat& Other) const;
    // (쿼터니언) 벡터 회전
    FVector RotateVector(const FVector& V) const;
    // 단위 쿼터니언 여부 확인
    bool IsNormalized() const;
    // 쿼터니언 정규화 (단위 쿼터니언으로 만듬)
    void Normalize();
    FQuat GetNormalized() const;
    // 회전 각도와 축으로부터 쿼터니언 생성 (axis-angle 방식)
    static FQuat FromAxisAngle(const FVector& Axis, float Angle);
    // 쿼터니언을 회전 행렬로 변환
    FMatrix ToMatrix() const;
    bool Equals(const FQuat& Q, float Tolerance = KINDA_SMALL_NUMBER) const;
    FRotator Rotator() const;
    static FQuat EulerToQuaternion(FVector Euler);
    static FVector QuaternionToEuler(const FQuat& Quat);
    static FQuat AxisAngleToQuaternion(const FVector& Axis, float AngleInDegrees);
    static FQuat AddQuaternions(const FQuat& q1, const FQuat& q2);
    static FQuat MultiplyQuaternions(const FQuat& q1, const FQuat& q2);
    static FQuat SubtractQuaternions(const FQuat& q1, const FQuat& q2);
    static FQuat MakeFromRotationMatrix(const FMatrix& M);
    FQuat GetInverse() const;
    static FVector4 VectorQuaternionRotateVector(const FQuat& Quat, FVector4 VectorW0);
    static FVector4 VectorQuaternionInverseRotatedVector(const FQuat& Q, const FVector4& W0);
    FVector GetEuler() const;
    FQuat Normalized() const;
};
inline FArchive& operator<<(FArchive& Ar, FQuat& Q)
{
    return Ar << Q.X << Q.Y << Q.Z << Q.W;
}
