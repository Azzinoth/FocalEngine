#pragma once

#include "../Core/Geometry/FELine.h"
#include <optional>

namespace FocalEngine
{
	template<typename T = float>
	class FEPlane
	{
	public:
		// Creates XY plane passing through origin.
		FEPlane() : Normal(0, 0, 1), PointOnPlane(0, 0, 0), GeneralFormCoefficients(0, 0, 1, 0) {};

		// Construct from point and normal vector (Point-Normal form).
		FEPlane(const glm::vec<3, T, glm::defaultp>& Point, const glm::vec<3, T, glm::defaultp>& Normal)
		{
			SetFromPointNormal(Point, Normal);
		}

		// Construct from general form coefficients ax + by + cz + d = 0.
		FEPlane(T A, T B, T C, T D)
		{
			SetFromGeneral(A, B, C, D);
		}

		// Construct from general form as Vec4(a, b, c, d).
		FEPlane(const glm::vec<4, T, glm::defaultp>& GeneralFormCoefficients)
		{
			SetFromGeneral(GeneralFormCoefficients);
		}

		// Construct from three points.
		FEPlane(const glm::vec<3, T, glm::defaultp>& Point_0, const glm::vec<3, T, glm::defaultp>& Point_1, const glm::vec<3, T, glm::defaultp>& Point_2)
		{
			SetFromThreePoints(Point_0, Point_1, Point_2);
		}

		void SetFromPointNormal(const glm::vec<3, T, glm::defaultp>& Point, const glm::vec<3, T, glm::defaultp>& Normal)
		{
			this->Normal = glm::normalize(Normal);
			PointOnPlane = Point;

			GeneralFormCoefficients = glm::vec<4, T, glm::defaultp>(this->Normal, -glm::dot(this->Normal, PointOnPlane));
		}

		void SetFromGeneralFormCoefficients(T A, T B, T C, T D)
		{
			SetFromGeneralFormCoefficients(glm::vec<4, T, glm::defaultp>(A, B, C, D));
		}

		void SetFromGeneralFormCoefficients(const glm::vec<4, T, glm::defaultp>& GeneralForm)
		{
			glm::vec<3, T, glm::defaultp> Normal(GeneralForm.x, GeneralForm.y, GeneralForm.z);
			T Length = glm::length(Normal);

			if (Length < std::numeric_limits<T>::epsilon())
			{
				LOG.Add("Plane normal is zero vector", "FE_GEOMETRY", FE_LOG_ERROR);
				return;
			}

			// Normalize the coefficients
			this->Normal = Normal / Length;
			GeneralFormCoefficients = GeneralForm / Length;

			// Find a point on the plane
			// Pick the largest component of the normal to avoid division by small values
			int LargestComponent = 0;
			T MaxValue = std::abs(this->Normal.x);

			if (std::abs(this->Normal.y) > MaxValue)
			{
				LargestComponent = 1;
				MaxValue = std::abs(this->Normal.y);
			}

			if (std::abs(this->Normal.z) > MaxValue)
				LargestComponent = 2;

			PointOnPlane = glm::vec<3, T, glm::defaultp>(0);
			PointOnPlane[LargestComponent] = -GeneralFormCoefficients.w / this->Normal[LargestComponent];
		}

		void SetFromThreePoints(const glm::vec<3, T, glm::defaultp>& Point_0, const glm::vec<3, T, glm::defaultp>& Point_1, const glm::vec<3, T, glm::defaultp>& Point_2)
		{
			glm::vec<3, T, glm::defaultp> Vector_0 = Point_1 - Point_0;
			glm::vec<3, T, glm::defaultp> Vector_1 = Point_2 - Point_0;
			glm::vec<3, T, glm::defaultp> Normal = glm::normalize(glm::cross(Vector_0, Vector_1));

			SetFromPointNormal(Point_0, Normal);
		}

		glm::vec<3, T, glm::defaultp> GetNormal() const
		{
			return Normal;
		}

		glm::vec<3, T, glm::defaultp> GetPointOnPlane() const
		{
			return PointOnPlane;
		}

		glm::vec<4, T, glm::defaultp> GetGeneralFormCoefficients() const
		{
			return GeneralFormCoefficients;
		}

		// Calculate signed distance from a point to the plane.
		T SignedDistanceTo(const glm::vec<3, T, glm::defaultp>& Point) const
		{
			return glm::dot(Normal, Point) + GeneralFormCoefficients.w;
		}
		
		// Determine if a point lies on the plane.
		bool DoesContainsPoint(const glm::vec<3, T, glm::defaultp>& Point, T Epsilon = static_cast<T>(1e-6)) const
		{
			return std::abs(SignedDistanceTo(Point)) < Epsilon;
		}

		glm::vec<3, T, glm::defaultp> ProjectPoint(const glm::vec<3, T, glm::defaultp>& Point) const
		{
			T Distance = SignedDistanceTo(Point);
			return Point - Distance * Normal;
		}

		bool Equals(const FEPlane<T>& Other, T Epsilon = static_cast<T>(1e-6)) const
		{
			// Two planes are equal if they have parallel normals and the same distance from origin
			// First check if normals are parallel or anti-parallel
			bool bNormalAlignment = glm::all(glm::epsilonEqual(Normal, Other.Normal, Epsilon)) ||
								    glm::all(glm::epsilonEqual(Normal, -Other.Normal, Epsilon));

			if (!bNormalAlignment)
				return false;

			// Then check if a point from one plane lies on the other.
			return Other.DoesContainsPoint(PointOnPlane, Epsilon);
		}

		// Is not thoroughly tested.
		std::optional<glm::vec<3, T, glm::defaultp>> DoesIntersectRay(const glm::vec<3, T, glm::defaultp>& RayOrigin, const glm::vec<3, T, glm::defaultp>& RayDirection) const
		{
			T Denominator = glm::dot(Normal, RayDirection);

			// If denominator is zero, line is parallel to the plane.
			if (std::abs(Denominator) < std::numeric_limits<T>::epsilon())
			{
				return std::nullopt;
			}

			T IntersectionParameter = -(glm::dot(Normal, RayOrigin) + GeneralFormCoefficients.w) / Denominator;
			return RayOrigin + IntersectionParameter * RayDirection;
		}

		std::optional<glm::vec<3, T, glm::defaultp>> DoesIntersectLine(const glm::vec<3, T, glm::defaultp>& LineOrigin, const glm::vec<3, T, glm::defaultp>& SecondPointOnLine) const
		{
			// Calculate direction vector from the two points
			glm::vec<3, T, glm::defaultp> LineDirection = SecondPointOnLine - LineOrigin;

			T Denominator = glm::dot(Normal, LineDirection);

			// If denominator is zero, line is parallel to the plane
			if (std::abs(Denominator) < std::numeric_limits<T>::epsilon())
			{
				return std::nullopt;
			}

			T IntersectionParameter = -(glm::dot(Normal, LineOrigin) + GeneralFormCoefficients.w) / Denominator;
			return LineOrigin + IntersectionParameter * LineDirection;
		}

		// Intersection of 2-planes: a variation based on the 3-plane version.
		// see: Graphics Gems 1 pg 305
		std::optional<std::pair<glm::vec<3, T, glm::defaultp>, glm::vec<3, T, glm::defaultp>>> DoesIntersectPlane(const FEPlane<T>& Other) const
		{
			const glm::vec<3, T, glm::defaultp> IntersectionNormal = glm::cross(Normal, Other.GetNormal());
			const T Determinant = glm::length2(IntersectionNormal);

			// If the determinant is 0, that means parallel planes, no intersection.
			if (Determinant > std::numeric_limits<T>::epsilon())
			{
				glm::vec<3, T, glm::defaultp> FirstPoint = ((glm::cross(IntersectionNormal, Other.GetNormal()) * GeneralFormCoefficients.w) + (glm::cross(Normal, IntersectionNormal) * Other.GetGeneralFormCoefficients().w)) / Determinant;
				glm::vec<3, T, glm::defaultp> SecondPoint = FirstPoint + IntersectionNormal;

				return std::make_pair(FirstPoint, SecondPoint);
			}
			else
			{
				return std::nullopt;
			}
		}

		// Intersection of 3-planes.
		// see: 3D Math Primer for Graphics, page 726
		std::optional<glm::vec<3, T, glm::defaultp>> DoesIntersectPlanes(const FEPlane<T>& SecondPlane, const FEPlane<T>& ThirdPlane) const
		{
			const T Denominator = glm::dot(glm::cross(Normal, SecondPlane.GetNormal()), ThirdPlane.GetNormal());
			// If the Denominator is 0, that means parallel planes or not unique intersection.
			if (std::abs(Denominator) < std::numeric_limits<T>::epsilon())
				return std::nullopt;
			
			const glm::vec<3, T, glm::defaultp> FirstNumeratorPart = GeneralFormCoefficients.w * glm::cross(SecondPlane.GetNormal(), ThirdPlane.GetNormal());
			const glm::vec<3, T, glm::defaultp> SecondNumeratorPart = SecondPlane.GetGeneralFormCoefficients().w * glm::cross(ThirdPlane.GetNormal(), Normal);
			const glm::vec<3, T, glm::defaultp> ThirdNumeratorPart = ThirdPlane.GetGeneralFormCoefficients().w * glm::cross(Normal, SecondPlane.GetNormal());

			const glm::vec<3, T, glm::defaultp> IntersectionPoint = (FirstNumeratorPart + SecondNumeratorPart + ThirdNumeratorPart) / Denominator;
			// We negate the result because the mathematical formula for three-plane intersection is:
			// P = (-d1(N2×N3) - d2(N3×N1) - d3(N1×N2)) / ((N1×N2)·N3)
			// Our implementation uses the d terms with opposite sign convention from the formula
			// (i.e., our GeneralFormCoefficients.w represents -d in the standard formula)
			return -IntersectionPoint;
		}

	private:
		glm::vec<3, T, glm::defaultp> Normal;
		glm::vec<3, T, glm::defaultp> PointOnPlane;
		glm::vec<4, T, glm::defaultp> GeneralFormCoefficients;
	};
}