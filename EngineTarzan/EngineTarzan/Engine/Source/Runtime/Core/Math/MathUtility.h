#pragma once
#include <cmath>
#include <numbers>

#include <random>
#include "Core/HAL/PlatformType.h"


#define PI                   (3.1415926535897932f)
#define SMALL_NUMBER         (1.e-8f)
#define KINDA_SMALL_NUMBER   (1.e-4f)

#define PI_DOUBLE            (3.141592653589793238462643383279502884197169399)
#define UE_SMALL_NUMBER            (1.e-8f)

struct FMath
{
    /** A와 B중에 더 작은 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Min(const T A, const T B)
    {
        return A < B ? A : B;
    }

    /** A와 B중에 더 큰 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Max(const T A, const T B)
    {
        return B < A ? A : B;
    }

    /** A, B, C 중에 가장 큰 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Max3(const T A, const T B, const T C)
    {
        return Max(A, Max(B, C));
    }

    /** X를 Min과 Max의 사이의 값으로 클램핑 합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Clamp(const T X, const T MinValue, const T MaxValue)
    {
        return Max(Min(X, MaxValue), MinValue);
    }

    /** A의 절댓값을 구합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Abs(const T A)
    {
        return A < T(0) ? -A : A;
    }

    /** Returns 1, 0, or -1 depending on relation of T to 0 */
    template< class T > 
    static constexpr FORCEINLINE T Sign( const T A )
    {
        return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
    }

    /** A의 제곱을 구합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Pow(const T A)
    {
        return A * A;
    }

    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Pow(const T A, const T B)
    {
        return pow(A, B);
    }

    // A의 제곱근을 구합니다.
    [[nodiscard]] static FORCEINLINE float Sqrt(float A) { return sqrtf(A); }
    [[nodiscard]] static FORCEINLINE double Sqrt(double A) { return sqrt(A); }

    /** A의 역제곱근을 구합니다. */
    [[nodiscard]] static FORCEINLINE float InvSqrt(float A) { return 1.0f / sqrtf(A); }
    [[nodiscard]] static FORCEINLINE double InvSqrt(double A) { return 1.0 / sqrt(A); }

    /** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, float Alpha)
    {
        return static_cast<T>((A * (1.0f - Alpha)) + (B * Alpha));
    }

    /** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
    template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, double Alpha)
	{
		return static_cast<T>((A * (1.0 - Alpha)) + (B * Alpha));
	}

    /**
 *	Checks if a floating point number is nearly zero.
 *	@param Value			Number to compare
 *	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
 *	@return					true if Value is nearly zero
 */
    [[nodiscard]] static FORCEINLINE bool IsNearlyZero(float Value, float ErrorTolerance = UE_SMALL_NUMBER)
	{
	    return Abs<float>( Value ) <= ErrorTolerance;
	}
    
    /**
     * Performs a cubic interpolation
     *
     * @param  P - end points
     * @param  T - tangent directions at end points
     * @param  A - distance along spline
     *
     * @return  Interpolated value
     */
    template <
        typename T,
        typename U
    >
    [[nodiscard]] static constexpr FORCEINLINE T CubicInterp( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
	{
	    const U A2 = A * A;
	    const U A3 = A2 * A;

	    return T((((2*A3)-(3*A2)+1) * P0) + ((A3-(2*A2)+A) * T0) + ((A3-A2) * T1) + (((-2*A3)+(3*A2)) * P1));
	}

    /** Interpolate between A and B, applying an ease in function.  Exp controls the degree of the curve. */
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseIn(const T& A, const T& B, float Alpha, float Exp)
	{
	    float const ModifiedAlpha = Pow(Alpha, Exp);
	    return Lerp<T>(A, B, ModifiedAlpha);
	}

    /** Interpolate between A and B, applying an ease out function.  Exp controls the degree of the curve. */
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseOut(const T& A, const T& B, float Alpha, float Exp)
	{
	    float const ModifiedAlpha = 1.f - Pow(1.f - Alpha, Exp);
	    return Lerp<T>(A, B, ModifiedAlpha);
	}
    
    /** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
    template< class T > 
    [[nodiscard]] static FORCEINLINE T InterpEaseInOut( const T& A, const T& B, float Alpha, float Exp )
	{
	    return Lerp<T>(A, B, (Alpha < 0.5f) ?
            InterpEaseIn(0.f, 1.f, Alpha * 2.f, Exp) * 0.5f :
            InterpEaseOut(0.f, 1.f, Alpha * 2.f - 1.f, Exp) * 0.5f + 0.5f);
	}
    
    /** Interpolate float from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
    template<typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
    [[nodiscard]] static auto FInterpTo( T1  Current, T2 Target, T3 DeltaTime, T4 InterpSpeed )
	{
	    static_assert(!std::is_same_v<T1, bool> && !std::is_same_v<T2, bool>, "Boolean types may not be interpolated");
	    using RetType = decltype(T1() * T2() * T3() * T4());
	
	    // If no interp speed, jump to target value
	    if( InterpSpeed <= 0.f )
	    {
	        return static_cast<RetType>(Target);
	    }

	    // Distance to reach
	    const RetType Dist = Target - Current;

	    // If distance is too small, just set the desired location
	    if( FMath::Square(Dist) < UE_SMALL_NUMBER )
	    {
	        return static_cast<RetType>(Target);
	    }

	    // Delta Move, Clamp so we do not over shoot.
	    //const RetType DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
	    
	    // TODO: 현재 float * FVector가 float값으로 받는게 안되기때문에 float로 강제하는 중. 아래가 원래 Unreal Code
	    const RetType DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

	    return Current + DeltaMove;				
	}


    template<typename FVector>
    [[nodiscard]] static FVector VInterpTo(const FVector Current, const FVector Target, float DeltaTime, float InterpSpeed)
	{
        // If no interp speed, jump to target value
        if (InterpSpeed <= 0.f)
        {
            return Target;
        }

        // Distance to reach
        const FVector Dist = Target - Current;

        // If distance is too small, just set the desired location
        if (Dist.SizeSquared() < KINDA_SMALL_NUMBER)
        {
            return Target;
        }

        // Delta Move, Clamp so we do not over shoot.
        const FVector DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

        return Current + DeltaMove;
	}

    template <typename FRotator>
    [[nodiscard]] static FRotator RInterpTo(const FRotator Current, const FRotator Target, float DeltaTime, float InterpSpeed)
    {
        // if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
        if (DeltaTime == 0.f || Current == Target)
        {
            return Current;
        }

        // If no interp speed, jump to target value
        if (InterpSpeed <= 0.f)
        {
            return Target;
        }

        const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

        const FRotator Delta = (Target - Current).GetNormalized();

        // If steps are too small, just return Target and assume we have reached our destination.
        if (Delta.IsNearlyZero())
        {
            return Target;
        }

        // Delta Move, Clamp so we do not over shoot.
        const FRotator DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);
        return (Current + DeltaMove).GetNormalized();
    }

	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr auto RadiansToDegrees(const T& RadVal) -> decltype(RadVal * (180.0f / PI))
	{
		return RadVal * (180.0f / PI);
	}

	[[nodiscard]] static FORCEINLINE constexpr float RadiansToDegrees(float RadVal)
	{
		return RadVal * (180.0f / PI);
	}

	[[nodiscard]] static FORCEINLINE constexpr double RadiansToDegrees(double RadVal)
	{
		return RadVal * (180.0 / PI_DOUBLE);
	}

	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr auto DegreesToRadians(const T& DegVal) -> decltype(DegVal * (PI / 180.0f))
	{
		return DegVal * (PI / 180.0f);
	}

	[[nodiscard]] static FORCEINLINE constexpr float DegreesToRadians(float DegVal)
	{
		return DegVal * (PI / 180.0f);
	}

	[[nodiscard]] static FORCEINLINE constexpr double DegreesToRadians(double DegVal)
	{
		return DegVal * (PI_DOUBLE / 180.0);
	}

    // Returns e^Value
    static FORCEINLINE float Exp( float Value ) { return expf(Value); }
    static FORCEINLINE double Exp(double Value) { return exp(Value); }

    // Returns 2^Value
    static FORCEINLINE float Exp2( float Value ) { return powf(2.f, Value); /*exp2f(Value);*/ }
    static FORCEINLINE double Exp2(double Value) { return pow(2.0, Value); /*exp2(Value);*/ }

    static FORCEINLINE float Loge( float Value ) {	return logf(Value); }
    static FORCEINLINE double Loge(double Value) { return log(Value); }

    static FORCEINLINE float LogX( float Base, float Value ) { return Loge(Value) / Loge(Base); }
    static FORCEINLINE double LogX(double Base, double Value) { return Loge(Value) / Loge(Base); }

    // 1.0 / Loge(2) = 1.4426950f
    static FORCEINLINE float Log2( float Value ) { return Loge(Value) * std::numbers::log2e_v<float>; }	
    // 1.0 / Loge(2) = 1.442695040888963387
    static FORCEINLINE double Log2(double Value) { return Loge(Value) * std::numbers::log2e; }


	[[nodiscard]] static FORCEINLINE double Cos(double RadVal) { return cos(RadVal); }
	[[nodiscard]] static FORCEINLINE float Cos(float RadVal) { return cosf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Sin(double RadVal) { return sin(RadVal); }
	[[nodiscard]] static FORCEINLINE float Sin(float RadVal) { return sinf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Tan(double RadVal) { return tan(RadVal); }
	[[nodiscard]] static FORCEINLINE float Tan(float RadVal) { return tanf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Acos(double Value) { return acos(Value); }
	[[nodiscard]] static FORCEINLINE float Acos(float Value) { return acosf(Value); }

	[[nodiscard]] static FORCEINLINE double Asin(double Value) { return asin(Value); }
	[[nodiscard]] static FORCEINLINE float Asin(float Value) { return asinf(Value); }

	[[nodiscard]] static FORCEINLINE double Atan(double Value) { return atan(Value); }
	[[nodiscard]] static FORCEINLINE float Atan(float Value) { return atanf(Value); }

	[[nodiscard]] static FORCEINLINE double Atan2(double Y, double X) { return atan2(Y, X); }
	[[nodiscard]] static FORCEINLINE float Atan2(float Y, float X) { return atan2f(Y, X); }

	static FORCEINLINE void SinCos(float* ScalarSin, float* ScalarCos, float Value)
	{
		*ScalarSin = sinf(Value);
		*ScalarCos = cosf(Value);
	}

	static FORCEINLINE void SinCos(double* ScalarSin, double* ScalarCos, double Value)
	{
		*ScalarSin = sin(Value);
		*ScalarCos = cos(Value);
	}

    template <typename T>
	[[nodiscard]] static FORCEINLINE T Square(T Value) { return Value * Value; }


	[[nodiscard]] static FORCEINLINE int32 CeilToInt(float Value) { return static_cast<int32>(ceilf(Value)); }
	[[nodiscard]] static FORCEINLINE int32 CeilToInt(double Value) { return static_cast<int32>(ceil(Value)); }

    template <typename T>
    [[nodiscard]] static FORCEINLINE int32 CeilToInt(T Value) { return static_cast<int32>(ceil(Value)); }


	[[nodiscard]] static FORCEINLINE float UnwindDegrees(float A)
	{
        // 1) 360으로 나눈 나머지를 구하되, A가 음수일 때도 올바르게 처리
        A = Fmod(A + 180.0f, 360.0f);
        // 2) fmodf 결과가 음수면 한 바퀴(360도) 더해주고
        if (A < 0.0f)  
        {
            A += 360.0f;
        }
        // 3) 다시 -180~+180 범위로 이동
        return A - 180.0f;
	}

    [[nodiscard]] static float Fmod(float X, float Y)
	{
	    const float AbsY = Abs(Y);
	    if (AbsY <= SMALL_NUMBER)
	    {
	        return 0.0;
	    }

	    return fmodf(X, Y);
	}

    static int RandHelper(int max)
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, max - 1);
        return dist(rng);
    }

    static const int p[512];

    static float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    static float grad(int hash, float x) {
        int h = hash & 15;
        float grad = 1.0f + (h & 7); // Gradient value 1-8
        if (h & 8) grad = -grad;
        return grad * x;
    }

    static float PerlinNoise1D(float x) {
        int xi = static_cast<int>(std::floor(x)) & 255;
        float xf = x - std::floor(x);
        float u = fade(xf);
        int a = p[xi];
        int b = p[xi + 1];
        return lerp(grad(a, xf), grad(b, xf - 1.0f), u);
    }
};
