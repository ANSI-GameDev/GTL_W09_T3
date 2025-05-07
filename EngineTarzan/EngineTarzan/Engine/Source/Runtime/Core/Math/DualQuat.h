// Math/DualQuat.h
#pragma once

#include "Math/Quat.h"
#include "Math/Vector.h"
#include "Math/Matrix.h" // MatrixToQuatTrans_UE 용

struct FDualQuat
{
    FQuat Real;
    FQuat Dual;

    // 기본 생성자: 항등 듀얼 쿼터니언
    FDualQuat()
        : Real(0.f, 0.f, 0.f, 1.f) // FQuat::Identity
        , Dual(0.f, 0.f, 0.f, 0.f)
    {
    }

    FDualQuat(const FQuat& InReal, const FQuat& InDual)
        : Real(InReal)
        , Dual(InDual)
    {
    }

    FDualQuat(const FQuat& q0, const FVector& t)
        : Real(q0)
    {
        // FQuat (X,Y,Z,W)
        Dual.W = -0.5f * (t.X * q0.X + t.Y * q0.Y + t.Z * q0.Z);
        Dual.X = 0.5f * (t.X * q0.W + t.Y * q0.Z - t.Z * q0.Y);
        Dual.Y = 0.5f * (-t.X * q0.Z + t.Y * q0.W + t.Z * q0.X);
        Dual.Z = 0.5f * (t.X * q0.Y - t.Y * q0.X + t.Z * q0.W);
    }

    void ToQuatTrans(FQuat& OutQ0, FVector& OutT) const
    {
        OutQ0 = Real;
        OutT.X = 2.0f * (-Dual.W * Real.X + Dual.X * Real.W - Dual.Y * Real.Z + Dual.Z * Real.Y);
        OutT.Y = 2.0f * (-Dual.W * Real.Y + Dual.X * Real.Z + Dual.Y * Real.W - Dual.Z * Real.X);
        OutT.Z = 2.0f * (-Dual.W * Real.Z - Dual.X * Real.Y + Dual.Y * Real.X + Dual.Z * Real.W);
    }

    FDualQuat operator+(const FDualQuat& Other) const
    {
        // FQuat에 + 연산자가 정의되어 있지 않다면, FQuat::AddQuaternions 또는 직접 멤버별 덧셈
        return FDualQuat(
            FQuat(Real.X + Other.Real.X, Real.Y + Other.Real.Y, Real.Z + Other.Real.Z, Real.W + Other.Real.W),
            FQuat(Dual.X + Other.Dual.X, Dual.Y + Other.Dual.Y, Dual.Z + Other.Dual.Z, Dual.W + Other.Dual.W)
        );
    }

    FDualQuat operator*(float Scalar) const
    {
        // FQuat에 operator*(float)가 정의되어 있다고 가정
        return FDualQuat(Real * Scalar, Dual * Scalar);
    }

    void Normalize()
    {
        float RealMagSq = Real.X * Real.X + Real.Y * Real.Y + Real.Z * Real.Z + Real.W * Real.W;
        if (RealMagSq <= KINDA_SMALL_NUMBER)
        {
            Real = FQuat(0.f, 0.f, 0.f, 1.f); // Identity
            Dual = FQuat(0.f, 0.f, 0.f, 0.f);
            return;
        }
        float InvRealMag = FMath::InvSqrt(RealMagSq);

        Real = Real * InvRealMag; // Real 파트 정규화
        Dual = Dual * InvRealMag; // Dual 파트도 동일하게 스케일링

        // Dual 파트를 Real 파트에 직교하도록 조정
        float DotRD = FQuat::DotProduct(Real, Dual);
        FQuat Correction = Real * DotRD; // (Real · Dual_scaled) * Real
        Dual.X -= Correction.X;          // Dual_new = Dual_scaled - Correction
        Dual.Y -= Correction.Y;
        Dual.Z -= Correction.Z;
        Dual.W -= Correction.W;
    }

    FDualQuat GetNormalized() const
    {
        FDualQuat Result = *this;
        Result.Normalize();
        return Result;
    }

    FVector TransformPosition(const FVector& P) const
    {
        FQuat q_rot; // 기본 생성자 (Identity)
        FVector t_trans; // 기본 생성자 (Zero)
        ToQuatTrans(q_rot, t_trans);

        FVector RotatedP = q_rot.RotateVector(P);
        return RotatedP + t_trans;
    }

    FVector TransformVector(const FVector& V) const
    {
        return Real.RotateVector(V);
    }

    FVector4 TransformFVector4(const FVector4& V4) const
    {
        FVector V3(V4.X, V4.Y, V4.Z);
        FVector TransformedV3 = Real.RotateVector(V3);
        return FVector4(TransformedV3.X, TransformedV3.Y, TransformedV3.Z, V4.W);
    }

    // 듀얼 쿼터니언 곱: (r1,d1)*(r2,d2) = (r1*r2, r1*d2 + d1*r2)
    static FDualQuat MultiplyDual(const FDualQuat& A, const FDualQuat& B)
    {
        FQuat RealPart = FQuat::MultiplyQuaternions(A.Real, B.Real);
        FQuat DualPart1 = FQuat::MultiplyQuaternions(A.Real, B.Dual);
        FQuat DualPart2 = FQuat::MultiplyQuaternions(A.Dual, B.Real);
        FQuat DualSum = FQuat::AddQuaternions(DualPart1, DualPart2);
        return FDualQuat(RealPart, DualSum);
    }
};
