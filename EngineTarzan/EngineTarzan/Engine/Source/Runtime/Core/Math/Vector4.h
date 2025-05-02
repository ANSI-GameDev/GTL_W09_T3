#pragma once
#include "Vector.h"
#include "Serialization/Archive.h"


// 4D Vector
struct FVector4
{
    float X, Y, Z, W;
    
    static const FVector4 ONE;
    static const FVector4 ONENULL;

    static const FVector4 ZERO;
    static const FVector4 ZERONULL;
    static const FVector4 LEFT;
    static const FVector4 RIGHT;
    static const FVector4 UP;
    static const FVector4 DOWN;
    static const FVector4 FORWARD;
    static const FVector4 BACKWARD;

    static const FVector4 WHITE;
    static const FVector4 RED;
    static const FVector4 GREEN;
    static const FVector4 BLUE;
    static const FVector4 BLACK;

    // 두 개의 채널이 1인 색상들
    static const FVector4 YELLOW ;    // 빨강 + 초록 = 노랑
    static const FVector4 CYAN;      // 초록 + 파랑 = 청록색/시안
    static const FVector4 MAGENTA ;   // 빨강 + 파랑 = 자홍색/마젠타
	
    // 추가적인 색
    static const FVector4 ORANGE;    // 주황색
    static const FVector4 PURPLE;    // 보라색
    static const FVector4 TEAL;

    FVector4() : X(0), Y(0), Z(0), W(0) {}
    FVector4(float InX, float InY, float InZ, float InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {}
    FVector4(FVector InVector, float InW = 0)
        : X(InVector.X), Y(InVector.Y), Z(InVector.Z)
        , W(InW)
    {}
    FVector4(const FString& SourceString)
        : X(0), Y(0), Z(0), W(0)
    {
        InitFromString(SourceString);
    }

    FVector4 operator+(const FVector4& Other) const;
    FVector4 operator-(const FVector4& Other) const;

    FVector4 operator/(float Scalar) const;

    static FVector4 MultiplyVector4(const FVector4& a, const FVector4& b);
    
    FVector4 operator*(float Scalar) const;
    
    static FVector4 VectorQuaternionRotateVector(const FVector4& Q, const FVector4& V);
    static FVector4 CrossProduct(const FVector4& A, const FVector4& B);

    FORCEINLINE static FVector4 VectorMultiplyAdd(const FVector4& A, const FVector4& B, const FVector4& C)
    {
        return { A.X * B.X + C.X, A.Y * B.Y + C.Y, A.Z * B.Z + C.Z, A.W * B.W + C.W};
    }

    FORCEINLINE static FVector4 VectorMax(const FVector4& A, const FVector4& B)
    {
        return { FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y), FMath::Max(A.Z, B.Z), FMath::Max(A.W, B.W) };
    }	

    FString ToString() const;
    bool InitFromString(const FString& InSourceString);
};

inline FVector4 FVector4::operator-(const FVector4& Other) const
{
    return {
        X - Other.X,
        Y - Other.Y,
        Z - Other.Z,
        W - Other.W
    };
}

inline FVector4 FVector4::operator+(const FVector4& Other) const
{
    return {
        X + Other.X,
        Y + Other.Y,
        Z + Other.Z,
        W + Other.W
    };
}

inline FVector4 FVector4::operator/(float Scalar) const
{
    return {
        X / Scalar,
        Y / Scalar,
        Z / Scalar,
        W / Scalar
    };
}



inline FArchive& operator<<(FArchive& Ar, FVector4& V)
{
    return Ar << V.X << V.Y << V.Z << V.W;
}
