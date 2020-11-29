
// header gaurd
#ifndef _HEADER_GAURD_MATHEXT_HPP
#define _HEADER_GAURD_MATHEXT_HPP

// required
#define _USE_MATH_DEFINES

// main includes
#include <math.h>
#include <Types.hpp>

// a generic point structure
template<class T>
struct POINT2T
{
    // the unions here alow for two different representations of a POINT:
    // 1) as a (x,y) pair, or 2) as (m,b) pair from a line segment

    union {
        T x;    // x coordinate
        T m;    // slope
    };
    union {
        T y;    // y coordinate
        T b;    // intercept
    };

    // no values
    POINT2T(): x(0), y(0) {}

    // trivial
    template<class A, class B>
    POINT2T(const A& a, const B& b): x(T(a)), y(T(b)) {}

    // trivial
    template<class A>
    POINT2T(const POINT2T<A>& P): x(P.x), y(P.y) {}

};

// predef some stuff
typedef POINT2T<FLOAT>  POINT2F;
typedef POINT2T<DOUBLE> POINT2D;
typedef POINT2T<INT32>  POINT2I;


// for evaluation of which goes next
template<typename T>
BOOLEEN operator< ( const POINT2T<T>& lhs, const POINT2T<T>& rhs )
{
    if (lhs.x < rhs.x) return true;
    if (lhs.x < rhs.x) return true;
    return false;
}


// protected
namespace MATHEXT
{
    // advanced algorithms
    namespace ADVANCED
    {
        class DELAUNAY_TRIANGULATION;


    };




    template<class T1, class T2, class T3>
    BOOLEEN Inside(const POINT2T<T1>& S, const POINT2T<T2>& E, const POINT2T<T3>& M)
    {
        return ( (( M.x >= S.x && M.x < E.x) || ( M.x >= E.x && M.x < S.x) ) && (( M.y >= S.y && M.y < E.y) || ( M.y >= E.y && M.y < S.y) ) );
    }

    template<class T1, class T2, class T3>
    BOOLEEN InsideInclusive(const POINT2T<T1>& S, const POINT2T<T2>& E, const POINT2T<T3>& M)
    {
        return ( (( M.x >= S.x && M.x <= E.x) || ( M.x >= E.x && M.x <= S.x) ) && (( M.y >= S.y && M.y <= E.y) || ( M.y >= E.y && M.y <= S.y) ) );
    }

    template<class T1>
    BOOLEEN IsRectDegenerate(const POINT2T<T1>* P)
    {
        return (P[0].x == P[1].x) || (P[0].y == P[1].y);
    }

    // returns true if test and SRC intersect -> and returns the intersection points
    template<class T1, class T2, class T3>
    BOOLEEN InsideRect(const POINT2T<T1>* Src, const POINT2T<T2>* Test, POINT2T<T3>* Inter)
    {
        Inter[0].x = std::max(Src[0].x, Test[0].x);
        Inter[0].y = std::max(Src[0].y, Test[0].y);
        Inter[1].x = std::min(Src[1].x, Test[1].x);
        Inter[1].y = std::min(Src[1].y, Test[1].y);
        return (Inter[0].x >= Src[0].x && Inter[0].x <= Src[1].x) && 
               (Inter[0].y >= Src[0].y && Inter[0].y <= Src[1].y) && 
               (Inter[1].x >= Src[0].x && Inter[1].x <= Src[1].x) && 
               (Inter[1].y >= Src[0].y && Inter[1].y <= Src[1].y);
    }

    // returns true if test and SRC intersect -> no returned intersection points
    template<class T3, class T1, class T2>
    BOOLEEN InsideRect(const POINT2T<T1>* Src, const POINT2T<T2>* Test)
    {
        POINT2T<T3> Inter[2];
        Inter[0].x = std::max(Src[0].x, Test[0].x);
        Inter[0].y = std::max(Src[0].y, Test[0].y);
        Inter[1].x = std::min(Src[1].x, Test[1].x);
        Inter[1].y = std::min(Src[1].y, Test[1].y);
        return (Inter[0].x >= Src[0].x && Inter[0].x <= Src[1].x) && 
               (Inter[0].y >= Src[0].y && Inter[0].y <= Src[1].y) && 
               (Inter[1].x >= Src[0].x && Inter[1].x <= Src[1].x) && 
               (Inter[1].y >= Src[0].y && Inter[1].y <= Src[1].y);
    }

    template<class T1, class T2, class T3 = DOUBLE>
    T3 GetScalarDistanceSquaredBetweenPoints(const POINT2T<T1>& L, const POINT2T<T2>& R)
    {
        return (R.x-L.x)*(R.x-L.x) + (R.y-L.y)*(R.y-L.y);
    }
    template<class T1, class T2>
    DOUBLE GetScalarDistanceBetweenPoints(const POINT2T<T1>& L, const POINT2T<T2>& R)
    {
        return sqrt( GetScalarDistanceSquaredBetweenPoints(L,R) );
    }

    // gets the line that connects left and right, in y=mx+b
    template<class T1, class T2>
    POINT2D GetLineBetweenPoints(const POINT2T<T1> &Left, const POINT2T<T2>& Right)
    {
        DOUBLE M1 = (Right.y - Left.y)*1.0/(Right.x - Left.x); //if ((Right.x - Left.x) == 0) qDebug() << "DIV by 0 A";
        DOUBLE B1 = Left.y - M1*Left.x;

        // return the line between
        return POINT2D(M1, B1);
    }

    // gets the line that connects left and right, in x=(ny+b)
    template<class T1, class T2>
    POINT2D GetLineInverseBetweenPoints(const POINT2T<T1> &Left, const POINT2T<T2>& Right)
    {
        DOUBLE M1 = (Right.x - Left.x)*1.0/(Right.y - Left.y); //if ((Right.x - Left.x) == 0) qDebug() << "DIV by 0 A";
        DOUBLE B1 = Left.x - M1*Left.y;

        // return the line between
        return POINT2D(M1, B1);
    }

    // intersects of two lines returns a point (x,y)
    template<class T1, class T2>
    POINT2D GetPointIntersectionOfLines(const POINT2T<T1>& LineA, const POINT2T<T2>& LineB)
    {
        // equate the lines and return a point interesection
        DOUBLE Xp = (LineB.b - LineA.b)*1.0/(LineA.m - LineB.m);
        DOUBLE Yp = LineA.m*Xp + LineA.b;

        // return the coordinate
        return POINT2D(Xp, Yp);
    }

    // intersects of two lines returns a point (x,y)
    template<class T1, class T2>
    POINT2D GetPointIntersectionOfLinesInverse(const POINT2T<T1>& LineA, const POINT2T<T2>& LineB)
    {
        // equate the lines and return a point interesection
        DOUBLE Yp = (LineB.b - LineA.b)*1.0/(LineA.m - LineB.m);
        DOUBLE Xp = LineA.m*Yp + LineA.b;

        // return the coordinate
        return POINT2D(Xp, Yp);
    }

    // uses inverse slopes if needed, special case for exactly |- lines
    template<class T1, class T2>
    POINT2D GetPointIntersectionOfSegmentsSafe(const POINT2T<T1>* LineB, const POINT2T<T2>* LineA)
    {
        BOOLEEN DelX[2] = { (LineB[1].x - LineB[0].x) == 0.0, (LineA[1].x - LineA[0].x) == 0.0 };
        BOOLEEN DelY[2] = { (LineB[1].y - LineB[0].y) == 0.0, (LineA[1].y - LineA[0].y) == 0.0 };

        // check corner cases
        if (DelX[0] && DelY[1])     
        {
            return {LineB[0].x, LineA[1].y};
        }
        if (DelX[1] && DelY[0]) 
        {
            return {LineA[1].x, LineB[0].y};
        }
        if (DelX[0] || DelX[1]) // remaining cases
        {
            // flip both cus who cares
            return GetPointIntersectionOfLinesInverse( GetLineInverseBetweenPoints(LineA[0], LineA[1]), GetLineInverseBetweenPoints( LineB[0], LineB[1]) );
        }
        return GetPointIntersectionOfLines( GetLineBetweenPoints(LineA[0], LineA[1]), GetLineBetweenPoints( LineB[0], LineB[1]) );
    }

    template<class T1, class T2, class T3>
    BOOLEEN GetPointIntersectionOfSegments(const POINT2T<T1>* LineB, const POINT2T<T2>* LineA, POINT2T<T3> &Out)
    {
        auto onSegment = [](POINT2D p, POINT2D q, POINT2D r) -> bool 
        {
            if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
                q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
               return true;
 
            return false;
        };
        auto orientation = [](POINT2D p, POINT2D q, POINT2D r) -> int
        {
            // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
            // for details of below formula.
            double val = (q.y - p.y) * (r.x - q.x) -(q.x - p.x) * (r.y - q.y);
 
            if (val == 0) return 0;  // colinear
            return (val > 0)? 1: 2; // clock or counterclock wise
        };
        auto doIntersect = [&onSegment, &orientation](POINT2D p1, POINT2D q1, POINT2D p2, POINT2D q2) -> bool
        {
            // Find the four orientations needed for general and
            // special cases
            int o1 = orientation(p1, q1, p2);
            int o2 = orientation(p1, q1, q2);
            int o3 = orientation(p2, q2, p1);
            int o4 = orientation(p2, q2, q1);
 
            // General case
            if (o1 != o2 && o3 != o4)
                return true;
 
            // Special Cases
           // if (o1 == 0) return false; 
           // if (o1 == 0 && onSegment(p1, p2, q1)) return true; // p1, q1 and p2 are colinear and p2 lies on segment p1q1
           // if (o2 == 0 && onSegment(p1, q2, q1)) return true; // p1, q1 and q2 are colinear and q2 lies on segment p1q1
           // if (o3 == 0 && onSegment(p2, p1, q2)) return true; // p2, q2 and p1 are colinear and p1 lies on segment p2q2
           // if (o4 == 0 && onSegment(p2, q1, q2)) return true; // p2, q2 and q1 are colinear and q1 lies on segment p2q2
 
            //if (onSegment(p1, p2, q1))


            return false; // Doesn't fall in any of the above cases
        };

        BOOLEEN Intersection = doIntersect(LineA[0], LineA[1], LineB[0], LineB[1]);
        if (Intersection == true)
        {
            Out = GetPointIntersectionOfSegmentsSafe( LineA, LineB );
        }
        return Intersection;

    }

    // MIDPOINT of two points
    template<class T1, class T2>
    POINT2D GetMidpoint(const POINT2T<T1>& A, const POINT2T<T2>& B)
    {
        return POINT2D( (A.x + B.x)/2.0, (A.y + B.y)/2.0);
    }


    // gives the perendicular intersection of line A with point M, returns the intersection along A,
    template<class T1, class T2>
    POINT2D GetPointIntersectPerpendicularFromPoint(const POINT2T<T1>& A, const POINT2T<T2>& Middle)
    {
        DOUBLE M1 = A.x;
        DOUBLE B1 = A.y;
        if (fabs(M1) <  0.00001)
        {
            return POINT2D(Middle.x, B1);
        }
        else
        {
            DOUBLE M2 = -1.0 / M1;
            DOUBLE B2 = Middle.y - M2*Middle.x;
            DOUBLE Xp = (B1 - B2) / (M2 - M1);
            DOUBLE Yp = M1*Xp + B1;
        
            // give back the point
            return POINT2D(Xp, Yp);
        }
    }

    // rotates End towards the X axis (normalized at Start), pass in Slope = -Segment(Start,End).m
    template<class T1, class T2>
    POINT2D GetPointRotatedToXAxis(const POINT2T<T1>& Start, const POINT2T<T2>& End, DOUBLE Slope)
    {
        DOUBLE &M = Slope;
        DOUBLE xNew = Start.x + ((End.x-Start.x) - (End.y-Start.y)*M)/sqrt(1 + M*M);
        DOUBLE yNew = Start.y;// + ((End.y-Start.y) + (End.x-Start.x)*M)/sqrt(1 + M*M); // should be 0, or close
        return POINT2D(xNew, yNew);
    }





};

#endif // _HEADER_GAURD_MATHEXT_HPP