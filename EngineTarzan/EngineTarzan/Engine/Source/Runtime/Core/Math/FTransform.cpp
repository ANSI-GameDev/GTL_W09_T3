#include "FTransform.h"

#include "JungleMath.h"

FMatrix FTransform::GetViewMatrix() const
{
    return JungleMath::CreateViewMatrix(Position, Position + GetForward(), GetUp());
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
    return FVector::CrossProduct(FVector(0, 0, 1), GetForward()).GetSafeNormal();
}

FVector FTransform::GetUp() const
{
    return FVector::CrossProduct(GetForward(), GetRight()).GetSafeNormal();
}

FVector FTransform::GetLocalRight() const
{
    const FMatrix RotationMatrix = FMatrix::GetRotationMatrix(Rotation);
    const FVector Right = FVector(
        RotationMatrix.M[0][1],
        RotationMatrix.M[1][1],
        RotationMatrix.M[2][1]
    );
    return Right.GetSafeNormal();
}

FVector FTransform::GetLocalUp() const
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

void FTransform::Rotate(const FVector& InRotation)
{
    RotateRoll(InRotation.X);
    RotatePitch(InRotation.Y);
    RotateYaw(InRotation.Z);
}

void FTransform::RotateYaw(const float Angle)
{
    const FVector Axis = FVector(0, 0, 1);
    // 현재 회전에 Z축을 기준으로 Angle 만큼 회전하는 쿼터니언을 곱함
    Rotation = FQuat::MultiplyQuaternions(Rotation, FQuat(Axis, Angle));
}

void FTransform::RotatePitch(const float Angle)
{
    const FVector Axis = FVector(0, 1, 0).GetSafeNormal();
    Rotation = FQuat::MultiplyQuaternions(Rotation, FQuat(Axis, Angle));
}

void FTransform::RotateRoll(const float Angle)
{
    const FVector Axis = FVector(1, 0, 0).GetSafeNormal();
    Rotation = FQuat::MultiplyQuaternions(Rotation, FQuat(Axis, Angle));
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
    Rotation = quat;

    // Translation(이동) 추출
    const FVector translation = matrix.GetTranslation();
    Position = translation;
}

FTransform FTransform::MultiPly(const FTransform& A, const FTransform& B)
{
    FTransform Result;

    const FQuat QuatA = A.GetRotation();
    const FQuat QuatB = B.GetRotation();
    const FVector TranslationA = A.GetPosition();
    const FVector TranslationB = B.GetPosition();
    const FVector ScaleA = A.GetScale();
    const FVector ScaleB = B.GetScale();

    // 회전 합성: A의 회전 후 B의 회전을 적용
    Result.Rotation = FQuat::MultiplyQuaternions(QuatA, QuatB);

    // Translation 합성:
    // A.Translation에 B.Scale을 적용한 후, B의 회전으로 회전시키고, B.Translation을 더함.
    const FVector ScaledTransA = TranslationA * ScaleB;
    const FVector4 RotatedTranslate = FQuat::VectorQuaternionRotateVector(QuatB, FVector4(ScaledTransA, 0));
    Result.Position = FVector(RotatedTranslate.X + TranslationB.X, RotatedTranslate.Y + TranslationB.Y, RotatedTranslate.Z + TranslationB.Z);

    // 스케일 합성 (성분별 곱셈)
    Result.Scale = ScaleA * ScaleB;

    return Result;
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
    const FVector4 InputVectorW0 = FVector4(Vector, 0.0f);
    const FQuat NormalizedQuat = Rotation.Normalized();

    const FVector4 scaledVector = FVector4(InputVectorW0.X * Scale.X, InputVectorW0.Y * Scale.Y, InputVectorW0.Z * Scale.Z, 0.0f);
    const FVector4 rotatedVector = FQuat::VectorQuaternionRotateVector(NormalizedQuat, scaledVector);
    const FVector4 translatedVector = FVector4(rotatedVector.X + Position.X, rotatedVector.Y + Position.Y, rotatedVector.Z + Position.Z, 1.0f);
    return FVector(translatedVector.X, translatedVector.Y, translatedVector.Z);
}

FVector FTransform::TransformPositionNoScale(const FVector& Vector) const
{
    const FVector4 InputVectorW0 = FVector4(Vector, 0.0f);

    const FVector4 rotatedVector = FQuat::VectorQuaternionRotateVector(Rotation, InputVectorW0);
    const FVector4 translatedVector = FVector4(rotatedVector.X + Position.X, rotatedVector.Y + Position.Y, rotatedVector.Z + Position.Z, 1.0f);

    return FVector(translatedVector.X, translatedVector.Y, translatedVector.Z);
}

FVector FTransform::TransformVector(const FVector& Vector) const
{
    const FVector scaledVec3 = Vector * Scale;
    const FVector rotatedVec3 = Rotation.RotateVector(scaledVec3);
    return rotatedVec3;
}

FVector FTransform::TransformVectorNoScale(const FVector& Vector) const
{
    const FVector rotatedVec3 = Rotation.RotateVector(Vector);
    return rotatedVec3;
}

FVector FTransform::TransformVectorNoRotation(const FVector& Vector) const
{
    const FVector scaledVec3 = Vector * Scale;
    return scaledVec3;
}

FVector FTransform::InverseTransformPosition(const FVector& Vector) const
{
    // Translation 제거
    const FVector InputVec = Vector;

    const FVector translatedVec = InputVec - Position;

    const FVector4 VR = FQuat::VectorQuaternionInverseRotatedVector(Rotation, FVector4(translatedVec, 0.0f));

    const FVector VResult = FVector(VR.X / Scale.X, VR.Y / Scale.Y, VR.Z / Scale.Z);

    return VResult;
}

FVector FTransform::InverseTransformPositionNoScale(const FVector& Vector) const
{
    const FVector temp = { Vector.X - Position.X, Vector.Y - Position.Y, Vector.Z - Position.Z };
    const FVector rotated = Rotation.GetInverse().RotateVector(temp);
    return rotated;
}

FVector FTransform::InverseTransformVector(const FVector& Vector) const
{
    const FVector rotated = Rotation.GetInverse().RotateVector(Vector);
    return { rotated.X / Scale.X, rotated.Y / Scale.Y, rotated.Z / Scale.Z };
}

FVector FTransform::InverseTransformVectorNoScale(const FVector& Vector) const
{
    const FVector rotated = Rotation.GetInverse().RotateVector(Vector);
    return rotated;
}

FQuat FTransform::TransformRotation(const FQuat& InRotation) const
{
    return FQuat::MultiplyQuaternions(Rotation, InRotation);
}

FQuat FTransform::InverseTransformRotation(const FQuat& InRotation) const
{
    return FQuat::MultiplyQuaternions(Rotation.GetInverse(), InRotation);
}

FTransform FTransform::GetRelativeTransform(const FTransform& Other) const
{
    FTransform Result;

    // Scale = S(A)/S(B)
    const FVector VScale3D = Scale / Other.Scale;

    //VQTranslation = (  ( T(A).X - T(B).X ),  ( T(A).Y - T(B).Y ), ( T(A).Z - T(B).Z), 0.f );
    const FVector VQTranslation = Position - Other.Position;

    // Inverse RotatedTranslation
    const FVector VR = Other.Rotation.GetInverse().RotateVector(VQTranslation);

    //Translation = 1/S(B)
    const FVector VTranslation = VR * Other.Scale;

    // Rotation = Q(B)(-1) * Q(A)	
    const FQuat VRotation = FQuat::MultiplyQuaternions(Other.Rotation.GetInverse(), Rotation);

    Result.Scale = VScale3D;
    Result.Position = VTranslation;
    Result.Rotation = VRotation;

    return Result;
}

FTransform FTransform::GetRelativeTransformReverse(const FTransform& Other) const
{
    FTransform Result;

    // 상대 스케일: S(B) / S(A)
    // (각 축별 나눗셈을 수행합니다.)
    const FVector RelativeScale(
        Other.Scale.X / this->Scale.X,
        Other.Scale.Y / this->Scale.Y,
        Other.Scale.Z / this->Scale.Z
    );

    // 상대 회전: Q_rel = Q(B) * Q(A)⁻¹
    // 정규화된 쿼터니언의 경우 Inverse는 Conjugate와 동일합니다.
    const FQuat InverseRotation = this->Rotation.GetInverse();
    const FQuat RelativeRotation = FQuat::MultiplyQuaternions(Other.Rotation, InverseRotation);

    // RotatedTranslation:
    // this->Translation에 상대 회전을 적용하여 회전된 벡터를 구합니다.
    FVector RotatedTranslation = RelativeRotation.RotateVector(this->Position);

    // 최종 Translation:
    // Other.Translation에서 상대 스케일(컴포넌트별 곱셈)한 RotatedTranslation을 뺍니다.
    const FVector RelativeTranslation(
        Other.Position.X - RelativeScale.X * RotatedTranslation.X,
        Other.Position.Y - RelativeScale.Y * RotatedTranslation.Y,
        Other.Position.Z - RelativeScale.Z * RotatedTranslation.Z
    );

    // 결과 할당
    Result.Scale = RelativeScale;
    Result.Rotation = RelativeRotation;
    Result.Position = RelativeTranslation;

    // (필요시, 결과 값에 NaN 검사를 추가할 수 있습니다.)
    // Result.DiagnosticCheckNaN_All();

    return Result;
}

FTransform FTransform::SetToRelativeTransform(const FTransform& myTransform, const FTransform& ParentTransform)
{
    FTransform Result;
    // 1. Scale: S(this) / S(Parent) (각 축별 나눗셈)
    Result.Scale = FVector(
        myTransform.Scale.X / ParentTransform.Scale.X,
        myTransform.Scale.Y / ParentTransform.Scale.Y,
        myTransform.Scale.Z / ParentTransform.Scale.Z
    );

    // 2. Translation:
    // 우선 부모의 Translation을 빼서 차이를 구합니다.
    const FVector Diff = myTransform.Position - ParentTransform.Position;

    // 부모의 회전의 역수를 구합니다 (정규화된 쿼터니언에서는 Conjugate가 Inverse와 동일)
    const FQuat InverseParentRot = ParentTransform.Rotation.GetInverse();

    // 차이에 부모 회전 역수를 적용하여 회전된 Translation을 구합니다.
    const FVector RotatedDiff = InverseParentRot.RotateVector(Diff);

    // 그리고 부모의 Scale로 나누어 보정합니다 (각 축별 나눗셈)
    Result.Position = FVector(
        RotatedDiff.X / ParentTransform.Scale.X,
        RotatedDiff.Y / ParentTransform.Scale.Y,
        RotatedDiff.Z / ParentTransform.Scale.Z
    );

    // 3. Rotation:
    // 상대 회전 = Inverse(Parent.Rotation) * this->Rotation
    Result.Rotation = FQuat::MultiplyQuaternions(InverseParentRot, myTransform.Rotation);
    return Result;
}

FTransform FTransform::GetRelativeTransformUsingMatrixWithScale(const FTransform* Base, const FTransform* Relative) const
{
    // 목표: 올바른 회전(Orientation)을 얻기 위해 행렬을 사용합니다.
    // 단, Translation은 여전히 스케일을 고려해야 합니다.

    // 1. Base와 Relative를 스케일을 포함한 행렬로 변환합니다.
    const FMatrix AM = Base->GetMatrix();
    const FMatrix BM = Relative->GetMatrix();

    // 2. 결합 스케일 계산
    //    Scale = S(Base) / S(Relative) (각 축별 나눗셈)
    const FVector DesiredScale(
        Base->Scale.X / Relative->Scale.X,
        Base->Scale.Y / Relative->Scale.Y,
        Base->Scale.Z / Relative->Scale.Z
    );

    // 3. BM의 역행렬을 구하고, AM과 곱하여 상대 행렬 M을 구합니다.
    const FMatrix InvBM = FMatrix::Inverse(BM);
    const FMatrix M = AM * InvBM;

    // 4. 구해진 행렬 M과 원하는 스케일(DesiredScale)을 이용해 OutTransform을 구성합니다.
    return ConstructTransformFromMatrixWithDesiredScale(M, DesiredScale);
}

FTransform FTransform::ConstructTransformFromMatrixWithDesiredScale(const FMatrix& InMatrix, const FVector& DesiredScale) const
{
    // 1. Translation 추출  
    //    Translation은 4번째 행의 앞 세 개 요소에 저장되어 있다고 가정합니다.
    const FVector Translation(
        InMatrix.M[3][0],
        InMatrix.M[3][1],
        InMatrix.M[3][2]
    );

    // 2. 회전 행렬 복원  
    //    상위 3x3 부분은 (스케일 * 회전) 행렬입니다.
    //    각 행 i (0,1,2)에 대해, DesiredScale의 해당 성분으로 나누어 정규화된 회전 행렬 R를 구합니다.
    FMatrix RotationMatrix;
    for (int i = 0; i < 3; ++i)
    {
        // 각 행에 대해 스케일 값 선택
        const float ScaleComponent = (i == 0) ? DesiredScale.X : (i == 1 ? DesiredScale.Y : DesiredScale.Z);

        // 스케일 값이 0이 아닐 경우에만 나눕니다.
        // (0이면 기본값 0을 유지)
        for (int j = 0; j < 3; ++j)
        {
            RotationMatrix.M[i][j] = (ScaleComponent != 0.0f) ? (InMatrix.M[i][j] / ScaleComponent) : 0.0f;
        }
    }
    // 나머지 행/열은 단위행렬로 설정
    RotationMatrix.M[0][3] = 0.0f;
    RotationMatrix.M[1][3] = 0.0f;
    RotationMatrix.M[2][3] = 0.0f;
    RotationMatrix.M[3][0] = 0.0f;
    RotationMatrix.M[3][1] = 0.0f;
    RotationMatrix.M[3][2] = 0.0f;
    RotationMatrix.M[3][3] = 1.0f;

    // 3. 회전 행렬로부터 쿼터니언 복원  
    FQuat Rotation = FQuat::MakeFromRotationMatrix(RotationMatrix);

    // 4. 결과 할당  
    FTransform OutTransform;
    OutTransform.Position = Translation;
    OutTransform.Rotation = Rotation;
    OutTransform.Scale = DesiredScale;

    return OutTransform;
}

FTransform FTransform::Inverse() const
{
    const FVector4 InverseScale = FVector4(1.f / Scale.X, 1.f / Scale.Y, 1.f / Scale.Z, 0.f);
    const FQuat InverseRotation = Rotation.GetInverse();

    const FVector4 ScaledTranslation = FVector4(InverseScale.X * Position.X, InverseScale.Y * Position.Y, InverseScale.Z * Position.Z, 0.f);
    const FVector4 RotatedTranslation = FQuat::VectorQuaternionRotateVector(InverseRotation, ScaledTranslation);
    const FVector InverseTranslation = FVector(-RotatedTranslation.X, -RotatedTranslation.Y, -RotatedTranslation.Z);

    return FTransform(InverseTranslation, InverseRotation, FVector(InverseScale.X, InverseScale.Y, InverseScale.Z));
}

void FTransform::ToMatrixInternal(FVector& OutDiagonals, FVector& OutAdds, FVector& OutSubtracts) const
{
	const FVector4 MyRotation = FVector4(Rotation.X, Rotation.Y, Rotation.Z, Rotation.W);
	const FVector4 RotationX2Y2Z2 = FVector4(MyRotation.X + MyRotation.X, MyRotation.Y + MyRotation.Y, MyRotation.Z + MyRotation.Z, 0.f);
	const FVector4 RotationXX2YY2ZZ2 = FVector4::MultiplyVector4(RotationX2Y2Z2, MyRotation);

	// The diagonal terms of the rotation matrix are:
	//   (1 - (yy2 + zz2)) * scale
	//   (1 - (xx2 + zz2)) * scale
	//   (1 - (xx2 + yy2)) * scale
	const FVector4 yy2xx2xx2 = FVector4(RotationXX2YY2ZZ2.Y, RotationXX2YY2ZZ2.X, RotationXX2YY2ZZ2.X, RotationXX2YY2ZZ2.X);
	const FVector4 zz2zz2yy2 = FVector4(RotationXX2YY2ZZ2.Z, RotationXX2YY2ZZ2.Z, RotationXX2YY2ZZ2.Y, RotationXX2YY2ZZ2.X);
	const FVector4 diagonalSum = yy2xx2xx2 + zz2zz2yy2;
	const FVector4 diagonals = FVector4(1.f - diagonalSum.X, 1.f - diagonalSum.Y, 1.f - diagonalSum.Z, 1.f - diagonalSum.W);
	OutDiagonals = FVector(diagonals.X * Scale.X, diagonals.Y * Scale.Y, diagonals.Z * Scale.Z);
	// Grouping the non-diagonal elements in the rotation block by operations:
	//    ((x*y2,y*z2,x*z2) + (w*z2,w*x2,w*y2)) * scale.xyz and
	//    ((x*y2,y*z2,x*z2) - (w*z2,w*x2,w*y2)) * scale.yxz
	// Rearranging so the LHS and RHS are in the same order as for +
	//    ((x*y2,y*z2,x*z2) - (w*z2,w*x2,w*y2)) * scale.yxz

	// RotBase = x*y2, y*z2, x*z2
	// RotOffset = w*z2, w*x2, w*y2
	const FVector4 xyx = FVector4(MyRotation.X, MyRotation.Y, MyRotation.X, MyRotation.X);
	const FVector4 y2z2z2 = FVector4(RotationX2Y2Z2.Y, RotationX2Y2Z2.Z, RotationX2Y2Z2.Z, RotationX2Y2Z2.X);
	const FVector4 RotBase = FVector4::MultiplyVector4(xyx, y2z2z2);

	const FVector4 www = FVector4(MyRotation.W, MyRotation.W, MyRotation.W, MyRotation.W);
	const FVector4 z2x2y2 = FVector4(RotationX2Y2Z2.Z, RotationX2Y2Z2.X, RotationX2Y2Z2.Y, RotationX2Y2Z2.X);
	const FVector4 RotOffset = FVector4::MultiplyVector4(www, z2x2y2);

	// Adds = (RotBase + RotOffset)*Scale3D :  (x*y2 + w*z2) * Scale3D.X , (y*z2 + w*x2) * Scale3D.Y, (x*z2 + w*y2) * Scale3D.Z
	// Subtracts = (RotBase - RotOffset)*Scale3DYZX :  (x*y2 - w*z2) * Scale3D.Y , (y*z2 - w*x2) * Scale3D.Z, (x*z2 - w*y2) * Scale3D.X
	const FVector4 Adds = FVector4(RotBase.X + RotOffset.X, RotBase.Y + RotOffset.Y, RotBase.Z + RotOffset.Z, RotBase.W + RotOffset.W);
	OutAdds = FVector(Adds.X * Scale.X, Adds.Y * Scale.Y, Adds.Z * Scale.Z);

	const FVector4 Scale3DYZXW = FVector4(Scale.Y, Scale.Z, Scale.X, 1.0f);
	const FVector4 Subtracts = FVector4(RotBase.X - RotOffset.X, RotBase.Y - RotOffset.Y, RotBase.Z - RotOffset.Z, RotBase.W - RotOffset.W);
	OutSubtracts = FVector(Subtracts.X * Scale3DYZXW.X, Subtracts.Y * Scale3DYZXW.Y, Subtracts.Z * Scale3DYZXW.Z);
}

FMatrix FTransform::ToMatrixWithScale() const
{
	FMatrix OutMatrix;
	FVector Diagonals, Adds, Subtracts;

	ToMatrixInternal(Diagonals, Adds, Subtracts);

	const FVector4 Diagonals4 = FVector4(Diagonals.X, Diagonals.Y, Diagonals.Z, 0.0f);

	// OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale.X;    // Diagonal.X
	// OutMatrix.M[0][1] = (xy2 + wz2) * Scale.X;             // Adds.X
	// OutMatrix.M[0][2] = (xz2 - wy2) * Scale.X;             // Subtracts.Z
	// OutMatrix.M[0][3] = 0.0f;                              // DiagonalsXYZ_W0.W
	const FVector4 AddX_DC_DiagX_DC = FVector4(Adds.X, Adds.X, Diagonals4.X, Diagonals4.X);
	const FVector4 SubZ_DC_DiagX_DC = FVector4(Subtracts.Z, Diagonals4.X, Diagonals4.W, Diagonals4.X);
	const FVector4 Row0 = FVector4(AddX_DC_DiagX_DC.Z, AddX_DC_DiagX_DC.X, SubZ_DC_DiagX_DC.X, SubZ_DC_DiagX_DC.Z);


	// OutMatrix.M[1][0] = (xy2 - wz2) * Scale.Y;             // Subtracts.X
	// OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale.Y;    // Diagonal.Y
	// OutMatrix.M[1][2] = (yz2 + wx2) * Scale.Y;             // Adds.Y
	// OutMatrix.M[1][3] = 0.0f;                            // DiagonalsXYZ_W0.W
	const FVector4 SubX_DC_DiagY_DC = FVector4(Subtracts.X, Subtracts.X, Diagonals4.Y, Diagonals4.X);
	const FVector4 AddY_DC_DiagW_DC = FVector4(Adds.Y, Adds.X, Diagonals4.W, Diagonals4.X);
	const FVector4 Row1 = FVector4(SubX_DC_DiagY_DC.X, SubX_DC_DiagY_DC.Z, AddY_DC_DiagW_DC.X, AddY_DC_DiagW_DC.Z);

	// OutMatrix.M[2][0] = (xz2 + wy2) * Scale.Z;             // Adds.Z
	// OutMatrix.M[2][1] = (yz2 - wx2) * Scale.Z;             // Subtracts.Y
	// OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale.Z;    // Diagonals.Z
	// OutMatrix.M[2][3] = 0.0f;                              // DiagonalsXYZ_W0.W
	const FVector4 AddZ_DC_SubY_DC = FVector4(Adds.Z, Subtracts.X, Diagonals4.Y, Diagonals4.X);
	const FVector4 Row2 = FVector4(AddZ_DC_SubY_DC.X, AddZ_DC_SubY_DC.Z, Diagonals4.Z, Diagonals4.W);

	// OutMatrix.M[3][0] = Translation.X;
	// OutMatrix.M[3][1] = Translation.Y;
	// OutMatrix.M[3][2] = Translation.Z;
	// OutMatrix.M[3][3] = 1.0f;
	const FVector4 Row3 = FVector4(Position.X, Position.Y, Position.Z, 1.0f);

	OutMatrix.M[0][0] = Row0.X; OutMatrix.M[0][1] = Row0.Y; OutMatrix.M[0][2] = Row0.Z; OutMatrix.M[0][3] = Row0.W;
	OutMatrix.M[1][0] = Row1.X;	OutMatrix.M[1][1] = Row1.Y; OutMatrix.M[1][2] = Row1.Z; OutMatrix.M[1][3] = Row1.W;
	OutMatrix.M[2][0] = Row2.X; OutMatrix.M[2][1] = Row2.Y; OutMatrix.M[2][2] = Row2.Z; OutMatrix.M[2][3] = Row2.W;
	OutMatrix.M[3][0] = Row3.X; OutMatrix.M[3][1] = Row3.Y; OutMatrix.M[3][2] = Row3.Z; OutMatrix.M[3][3] = Row3.W;

	return OutMatrix;
}


