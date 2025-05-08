#include "Quat.h"

#include "Vector.h"
#include "Matrix.h"

FQuat::FQuat(const FVector& Axis, const float Angle)
{
    const float HalfAngle = Angle * 0.5f;
    const float SinHalfAngle = FMath::Sin(HalfAngle);
    const float CosHalfAngle = FMath::Cos(HalfAngle);

    X = Axis.X * SinHalfAngle;
    Y = Axis.Y * SinHalfAngle;
    Z = Axis.Z * SinHalfAngle;
    W = CosHalfAngle;
}

FQuat::FQuat(const FMatrix& InMatrix)
{
    float S;
    // Check diagonal (trace)
    const float trace = InMatrix.M[0][0] + InMatrix.M[1][1] + InMatrix.M[2][2]; // 행렬의 Trace 값 (대각합)

    if (trace > 0.0f) 
    {
        const float InvS = FMath::InvSqrt(trace + 1.f);
        this->W = 0.5f * (1.f / InvS);
        S = 0.5f * InvS;

        this->X = ((InMatrix.M[1][2] - InMatrix.M[2][1]) * S);
        this->Y = ((InMatrix.M[2][0] - InMatrix.M[0][2]) * S);
        this->Z = ((InMatrix.M[0][1] - InMatrix.M[1][0]) * S);
    } 
    else 
    {
        // diagonal is negative
        int32 i = 0;

        if (InMatrix.M[1][1] > InMatrix.M[0][0])
            i = 1;

        if (InMatrix.M[2][2] > InMatrix.M[i][i])
            i = 2;

        static constexpr int32 nxt[3] = { 1, 2, 0 };
        const int32 j = nxt[i];
        const int32 k = nxt[j];
 
        S = InMatrix.M[i][i] - InMatrix.M[j][j] - InMatrix.M[k][k] + 1.0f;

        const float InvS = FMath::InvSqrt(S);

        float qt[4];
        qt[i] = 0.5f * (1.f / InvS);

        S = 0.5f * InvS;

        qt[3] = (InMatrix.M[j][k] - InMatrix.M[k][j]) * S;
        qt[j] = (InMatrix.M[i][j] + InMatrix.M[j][i]) * S;
        qt[k] = (InMatrix.M[i][k] + InMatrix.M[k][i]) * S;

        this->X = qt[0];
        this->Y = qt[1];
        this->Z = qt[2];
        this->W = qt[3];

    }
}

FQuat FQuat::operator*(const FQuat& Other) const
{
    return FQuat(
            W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
            W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
            W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W,
            W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z
        );
}

bool FQuat::operator==(const FQuat& Other) const
{
    return X == Other.X && Y == Other.Y && Z == Other.Z && W == Other.W;
}

bool FQuat::operator!=(const FQuat& Other) const
{
    return X != Other.X || Y != Other.Y || Z != Other.Z || W != Other.W;
}

FVector FQuat::RotateVector(const FVector& V) const
{
    // v' = q * v * q^(-1) 구현

    // 벡터를 순수 쿼터니언으로 변환 (실수부 0, 벡터부 V)
    const FQuat VectorQuat(V.X, V.Y, V.Z, 0.0f);
    const FQuat Conjugate = GetInverse();
    const FQuat temp = MultiplyQuaternions(*this, VectorQuat);

    // 쿼터니언 회전 수행: q * v * q^(-1)
    const FQuat Result = MultiplyQuaternions(temp, Conjugate);

    // 결과에서 벡터 부분만 추출
    return FVector(Result.X, Result.Y, Result.Z);
}

bool FQuat::IsNormalized() const
{
    return fabs(W * W + X * X + Y * Y + Z * Z - 1.0f) < 1e-6f;
}

void FQuat::Normalize()
{
    // 각 성분 제곱합 계산
    const float magSq = W*W + X*X + Y*Y + Z*Z;

    // 크기가 유의미하면 역제곱근으로 스케일링
    if (magSq > SMALL_NUMBER)
    {
        const float invMag = FMath::InvSqrt(magSq);
        X *= invMag;
        Y *= invMag;
        Z *= invMag;
        W *= invMag;
    }
    else
    {
        // 너무 작으면 단위 쿼터니언으로 리셋
        *this = FQuat(0.f, 0.f, 0.f, 1.f);
    }
}

FQuat FQuat::GetNormalized() const
{
    FQuat Result = *this;
    Result.Normalize();
    return Result;
}

FQuat FQuat::FromAxisAngle(const FVector& Axis, float Angle)
{
    float halfAngle = Angle * 0.5f;
    float sinHalfAngle = sinf(halfAngle);
    return FQuat(Axis.X * sinHalfAngle, Axis.Y * sinHalfAngle, Axis.Z * sinHalfAngle, FMath::Cos(halfAngle));
}

FMatrix FQuat::ToMatrix() const
{
    FMatrix R;
    const float x2 = X + X;    const float y2 = Y + Y;    const float z2 = Z + Z;
    const float xx = X * x2;   const float xy = X * y2;   const float xz = X * z2;
    const float yy = Y * y2;   const float yz = Y * z2;   const float zz = Z * z2;
    const float wx = W * x2;   const float wy = W * y2;   const float wz = W * z2;

    R.M[0][0] = 1.0f - (yy + zz);    R.M[1][0] = xy - wz;                R.M[2][0] = xz + wy;            R.M[3][0] = 0.0f;
    R.M[0][1] = xy + wz;            R.M[1][1] = 1.0f - (xx + zz);        R.M[2][1] = yz - wx;            R.M[3][1] = 0.0f;
    R.M[0][2] = xz - wy;            R.M[1][2] = yz + wx;				R.M[2][2] = 1.0f - (xx + yy);	R.M[3][2] = 0.0f;
    R.M[0][3] = 0.0f;				R.M[1][3] = 0.0f;					R.M[2][3] = 0.0f;                R.M[3][3] = 1.0f;

    return R;
}

bool FQuat::Equals(const FQuat& Q, float Tolerance) const
{
    return (FMath::Abs(X - Q.X) <= Tolerance && FMath::Abs(Y - Q.Y) <= Tolerance && FMath::Abs(Z - Q.Z) <= Tolerance && FMath::Abs(W - Q.W) <= Tolerance)
        || (FMath::Abs(X + Q.X) <= Tolerance && FMath::Abs(Y + Q.Y) <= Tolerance && FMath::Abs(Z + Q.Z) <= Tolerance && FMath::Abs(W + Q.W) <= Tolerance);
}

FRotator FQuat::Rotator() const
{
    // 1) 쿼터니언 정규화 보장
    FQuat Q = IsNormalized() ? *this : GetNormalized();

    // 2) 싱귤러리티 검사용 값 계산
    const float SingularityTest = Q.Z * Q.X - Q.W * Q.Y;
    const float YawY = 2.f * (Q.W * Q.Z + Q.X * Q.Y);
    const float YawX = 1.f - 2.f * (FMath::Square(Q.Y) + FMath::Square(Q.Z));

    // 3) 상수 선언
    static const float SINGULARITY_THRESHOLD = 0.4999995f;
    static const float RAD_TO_DEG = 180.f / PI;

    float Pitch, Yaw, Roll;

    if (SingularityTest < -SINGULARITY_THRESHOLD)
    {
        Pitch = -90.f;
        Yaw   = FMath::Atan2(YawY, YawX) * RAD_TO_DEG;
        Roll  = FRotator::NormalizeAxis(-Yaw - (2.f * FMath::Atan2(Q.X, Q.W) * RAD_TO_DEG));
    }
    else if (SingularityTest > SINGULARITY_THRESHOLD)
    {
        Pitch = 90.f;
        Yaw   = FMath::Atan2(YawY, YawX) * RAD_TO_DEG;
        Roll  = FRotator::NormalizeAxis(Yaw - (2.f * FMath::Atan2(Q.X, Q.W) * RAD_TO_DEG));
    }
    else
    {
        // 4) Asin 입력값 클램핑
        float ClampedSin = FMath::Clamp(2.f * SingularityTest, -1.f, 1.f);
        Pitch = FMath::Asin(ClampedSin) * RAD_TO_DEG;
        Yaw   = FMath::Atan2(YawY, YawX) * RAD_TO_DEG;
        Roll  = FMath::Atan2(
                    -2.f * (Q.W * Q.X + Q.Y * Q.Z),
                     1.f - 2.f * (FMath::Square(Q.X) + FMath::Square(Q.Y))
                ) * RAD_TO_DEG;
    }

    return FRotator(Pitch, Yaw, Roll);
}

FQuat FQuat::EulerToQuaternion(const FVector Euler)
{
    const float roll = FMath::DegreesToRadians(Euler.X);
    const float pitch = FMath::DegreesToRadians(Euler.Y);
    const float yaw = FMath::DegreesToRadians(Euler.Z);

    const float cr = FMath::Cos(roll * 0.5f);
    const float sr = FMath::Sin(roll * 0.5f);
    const float cp = FMath::Cos(pitch * 0.5f);
    const float sp = FMath::Sin(pitch * 0.5f);
    const float cy = FMath::Cos(yaw * 0.5f);
    const float sy = FMath::Sin(yaw * 0.5f);

    FQuat q;
    q.W = cr * cp * cy + sr * sp * sy;
    q.X = sr * cp * cy - cr * sp * sy;
    q.Y = cr * sp * cy + sr * cp * sy;
    q.Z = cr * cp * sy - sr * sp * cy;

    return q;
}

FVector FQuat::QuaternionToEuler(const FQuat& Quat)
{
    FVector angles;

    // roll (x-axis rotation)
    const float sinr_cosp = 2.0f * (Quat.W * Quat.X + Quat.Y * Quat.Z);
    const float cosr_cosp = 1.0f - 2.0f * (Quat.X * Quat.X + Quat.Y * Quat.Y);
    angles.X = FMath::Atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    const float sinp = FMath::Sqrt(1.0f + 2.0f * (Quat.W * Quat.Y - Quat.X * Quat.Z));
    const float cosp = FMath::Sqrt(1.0f - 2.0f * (Quat.W * Quat.Y - Quat.X * Quat.Z));
    angles.Y = 2 * FMath::Atan2(sinp, cosp) - PI / 2;

    // yaw (z-axis rotation)
    const float siny_cosp = 2.0f * (Quat.W * Quat.Z + Quat.X * Quat.Y);
    const float cosy_cosp = 1.0f - 2.0f * (Quat.Y * Quat.Y + Quat.Z * Quat.Z);
    angles.Z = FMath::Atan2(siny_cosp, cosy_cosp);

    angles.X = FMath::RadiansToDegrees(angles.X);
    angles.Y = FMath::RadiansToDegrees(angles.Y);
    angles.Z = FMath::RadiansToDegrees(angles.Z);

    return angles;
}

FQuat FQuat::AxisAngleToQuaternion(const FVector& Axis, float AngleInDegrees)
{
    const float AngleInRadians = FMath::DegreesToRadians(AngleInDegrees);
    const float HalfAngle = AngleInRadians * 0.5f;
    const float s = FMath::Sin(HalfAngle);
    return FQuat(
        Axis.X * s,
        Axis.Y * s,
        Axis.Z * s,
        FMath::Cos(HalfAngle)
    );
}

FQuat FQuat::AddQuaternions(const FQuat& q1, const FQuat& q2)
{
    return FQuat(
    q1.X + q2.X,
    q1.Y + q2.Y,
    q1.Z + q2.Z,
    q1.W + q2.W
    );
}

FQuat FQuat::MultiplyQuaternions(const FQuat& q1, const FQuat& q2)
{
    return FQuat(
    q1.W * q2.X + q1.X * q2.W + q1.Y * q2.Z - q1.Z * q2.Y, // X
    q1.W * q2.Y - q1.X * q2.Z + q1.Y * q2.W + q1.Z * q2.X, // Y
    q1.W * q2.Z + q1.X * q2.Y - q1.Y * q2.X + q1.Z * q2.W, // Z
    q1.W * q2.W - q1.X * q2.X - q1.Y * q2.Y - q1.Z * q2.Z  // W
    );
}

FQuat FQuat::SubtractQuaternions(const FQuat& q1, const FQuat& q2)
{
    return FQuat(
        q1.X - q2.X,
        q1.Y - q2.Y,
        q1.Z - q2.Z,
        q1.W - q2.W
    );
}

FQuat FQuat::MakeFromRotationMatrix(const FMatrix& M)
{
    FQuat Q;

    const float trace = M.M[0][0] + M.M[1][1] + M.M[2][2]; // 행렬의 Trace 값 (대각합)

    if (trace > 0.0f)
    {
        const float S = sqrtf(trace + 1.0f) * 2.0f; // S는 4배의 W
        Q.W = 0.25f * S;
        Q.X = (M.M[2][1] - M.M[1][2]) / S;
        Q.Y = (M.M[0][2] - M.M[2][0]) / S;
        Q.Z = (M.M[1][0] - M.M[0][1]) / S;
    }
    else
    {
        if (M.M[0][0] > M.M[1][1] && M.M[0][0] > M.M[2][2])
        {
            float S = sqrtf(1.0f + M.M[0][0] - M.M[1][1] - M.M[2][2]) * 2.0f;
            Q.W = (M.M[2][1] - M.M[1][2]) / S;
            Q.X = 0.25f * S;
            Q.Y = (M.M[0][1] + M.M[1][0]) / S;
            Q.Z = (M.M[0][2] + M.M[2][0]) / S;
        }
        else if (M.M[1][1] > M.M[2][2])
        {
            float S = sqrtf(1.0f + M.M[1][1] - M.M[0][0] - M.M[2][2]) * 2.0f;
            Q.W = (M.M[0][2] - M.M[2][0]) / S;
            Q.X = (M.M[0][1] + M.M[1][0]) / S;
            Q.Y = 0.25f * S;
            Q.Z = (M.M[1][2] + M.M[2][1]) / S;
        }
        else
        {
            float S = sqrtf(1.0f + M.M[2][2] - M.M[0][0] - M.M[1][1]) * 2.0f;
            Q.W = (M.M[1][0] - M.M[0][1]) / S;
            Q.X = (M.M[0][2] + M.M[2][0]) / S;
            Q.Y = (M.M[1][2] + M.M[2][1]) / S;
            Q.Z = 0.25f * S;
        }
    }

    return Q;
}

FQuat FQuat::GetInverse() const
{
    if (IsNormalized())
    {
        return FQuat(-X, -Y, -Z, W);
    }
    else
    {
        FQuat NormalizedQuat = GetNormalized();
        return FQuat(-NormalizedQuat.X, -NormalizedQuat.Y, -NormalizedQuat.Z, NormalizedQuat.W);
    }
}

FVector4 FQuat::VectorQuaternionRotateVector(const FQuat& Quat, const FVector4 VectorW0)
{
    const FVector4 Q = FVector4(Quat.X, Quat.Y, Quat.Z, Quat.W);
    const FVector4 QW = FVector4(Quat.Z, Quat.Z, Quat.Z, Quat.Z);
    FVector4 T = FVector4::CrossProduct(Q, VectorW0);
    T = FVector4(T.X + T.X, T.Y + T.Y, T.Z + T.Z, T.W + T.W);

    const FVector4 VTemp0 = FVector4::VectorMultiplyAdd(QW, T, VectorW0);
    const FVector4 VTemp1 = FVector4::CrossProduct(Q, T);
    const FVector4 Rotated = FVector4(VTemp0.X + VTemp1.X, VTemp0.Y + VTemp1.Y, VTemp0.Z + VTemp1.Z, VTemp0.W + VTemp1.W);
    return Rotated;
}

FVector4 FQuat::VectorQuaternionInverseRotatedVector(const FQuat& Q, const FVector4& W0)
{
    const FQuat QInv = FQuat(-Q.X, -Q.Y, -Q.Z, -Q.W);
    return VectorQuaternionRotateVector(QInv, W0);
}

FVector FQuat::GetEuler() const
{
    return QuaternionToEuler(*this);
}

FQuat FQuat::Normalized() const
{
    // 쿼터니언의 제곱 노름(크기의 제곱) 계산
    const float SqNorm = X * X + Y * Y + Z * Z + W * W;

    // 아주 작은 값인지 확인 (SMALL_NUMBER는 충분히 작은 값, 예: 1e-8f)
    if (SqNorm > SMALL_NUMBER)
    {
        // 역제곱근 계산
        const float InvNorm = 1.0f / FMath::Sqrt(SqNorm);
        return FQuat(X * InvNorm, Y * InvNorm, Z * InvNorm, W * InvNorm);
    }

    // 크기가 0에 가까우면 항등 쿼터니언 반환 (회전 없음)
    return FQuat(0.0f, 0.0f, 0.0f, 1.0f);
}

float FQuat::DotProduct(const FQuat& A, const FQuat& B)
{
    return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
}
