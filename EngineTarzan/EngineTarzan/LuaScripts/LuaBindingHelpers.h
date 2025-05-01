#pragma once
#include <sol/sol.hpp>
#include "Runtime/Core/Math/Vector.h"
#include "Runtime/Engine/UserInterface/Console.h"

#include "Engine/Engine.h"
#include "World/World.h"

namespace LuaBindingHelpers
{
    // FVector 타입 바인딩 함수
    inline void BindFVector(sol::state& Lua)
    {
        Lua.new_usertype<FVector>("FVector",
            sol::constructors<
            FVector(),               // 기본 생성자
            FVector(float, float, float)  // XYZ 지정 생성자
            >(),
            // 멤버 변수 노출
            "X", &FVector::X,
            "Y", &FVector::Y,
            "Z", &FVector::Z,

            // 덧셈
            sol::meta_function::addition,
            [](const FVector& a, const FVector& b) { return a + b; },

            // 뺄셈
            sol::meta_function::subtraction,
            [](const FVector& a, const FVector& b) { return a - b; },

            // 곱셈 (vector * scalar, scalar * vector)
            sol::meta_function::multiplication,
            sol::overload(
                [](const FVector& v, float s) { return v * s; },
                [](float s, const FVector& v) { return v * s; }
            ),

            // 나눗셈 (vector / scalar)
            sol::meta_function::division,
            [](const FVector& v, float s) { return v / s; }
        );
    }

    inline void BindFRotator(sol::state& Lua)
    {
        Lua.new_usertype<FRotator>("FRotator",
            // 생성자
            sol::constructors<
                FRotator(), 
                FRotator(float, float, float)
            >(),

            // 속성
            "Pitch", &FRotator::Pitch,
            "Yaw",   &FRotator::Yaw,
            "Roll",  &FRotator::Roll,

            // 연산자
            sol::meta_function::addition,      [](const FRotator& a, const FRotator& b){ return a + b; },
            sol::meta_function::subtraction,   [](const FRotator& a, const FRotator& b){ return a - b; }
        );
    }
    
    // UE_LOG 바인딩 함수
    inline void BindUE_LOG(sol::state& Lua)
    {
        Lua.set_function("UE_LOG",
            [](const std::string& Level, const std::string& Msg)
            {
                FString Converted = FString(Msg.c_str());
                if (Level == "Error")
                {
                    UE_LOG(LogLevel::Error, TEXT("%s"), *Converted);
                }
                else if (Level == "Warning")
                {
                    UE_LOG(LogLevel::Warning, TEXT("%s"), *Converted);
                }
                else
                {
                    UE_LOG(LogLevel::Display, TEXT("%s"), *Converted);
                }
            }
        );
    }

    // Lua print 함수 바인딩 (콘솔 + 화면)
    inline void BindPrint(sol::state& Lua)
    {
        Lua.set_function("print",
            [](const std::string& Msg)
            {
                // 로그에 출력
                UE_LOG(LogLevel::Error, TEXT("%s"), Msg.c_str());
                // 화면에 출력
                OutputDebugStringA(Msg.c_str()); // 디버그 창에 출력
            }
        );
    }

    inline void BindController(sol::state& Lua)
    {
        Lua.set_function("controller",
            [](const std::string& Key, std::function<void(float)> Callback)
            {
                //FString 주면 됨
                GEngine->ActiveWorld->GetPlayerController()->BindAction(FString(Key), Callback);
            }
        );
    }
}

namespace LuaDebugHelper
{
    /**
     * @brief 바인딩 전 sol::state 의 globals() 키를 TArray<FString>로 캡처
     */
    inline TArray<FString> CaptureGlobalNames(sol::state& Lua)
    {
        TArray<FString> names;
        sol::table G = Lua.globals();
        for (auto& kv : G)
        {
            if (kv.first.is<std::string>())
            {
                names.Add(FString(kv.first.as<std::string>().c_str()));
            }
        }
        return names;
    }

    /**
     * @brief 바인딩 후 globals()에서 before에 없던 새 키만 로그로 출력
     */
    inline void LogNewBindings(sol::state& Lua, const TArray<FString>& Before)
    {
        sol::table G = Lua.globals();
        for (auto& kv : G)
        {
            if (!kv.first.is<std::string>())
                continue;

            FString name = FString(kv.first.as<std::string>().c_str());
            // Before 배열에 포함되지 않은 경우만 출력
            if (!Before.Contains(name))
            {
                UE_LOG(LogLevel::Error,TEXT("Lua binding added: %s"), *name);
            }
        }
    }
}
