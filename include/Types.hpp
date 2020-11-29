///////////////////////////////////////////////////////////////
//! @file      Types.hpp
//! @details   Access to the types that we'll be using
//! @author    Robert Lengyel
//! @version   1.0
//! @date      Feb 2, 2018
//! @copyright Copyright (c) 2018, Robert Lengyel
///////////////////////////////////////////////////////////////

// header gaurd
#ifndef _HEADER_GAURD_ENGINEXG3_TYPES_HPP
#define _HEADER_GAURD_ENGINEXG3_TYPES_HPP

// standard types int and c++ headers
#include <stdint.h>
#include <string>
#include <vector>
#include <exception>
#include <map>
#include <unordered_map>
#include <tuple>
#include <list>
#include <utility>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <initializer_list>
#include <set>
#include <mutex>
#include <atomic>
#include <future>

// basic types
typedef uint32_t                UINT32;                 //!< typedef for `uint32_t`
typedef int32_t                 INT32;                  //!< typedef for `int32_t`
typedef uint64_t                UINT64;                 //!< typedef for `uint64_t`
typedef int64_t                 INT64;                  //!< typedef for `int64_t`
typedef size_t                  TSIZE;                  //!< typedef for `size_t`
typedef char                    CHAR;                   //!< typedef for `char`
typedef float                   FLOAT;                  //!< typedef for `float_t`
typedef double                  DOUBLE;                 //!< typedef for `double_t`
typedef bool                    BOOLEEN;                //!< typedef for `bool`. This was selected as BOOLEEN instead of BOOL to avoid conflicts with Microsoft BOOL which is INT underneath
typedef void*                   METADATA;               //!< typedef for `void*`
//typedef void                    VOID;                   //!< typedef for `void`

// opencv stuff
/*#ifdef __OPENCV_ALL_HPP__
typedef cv::Mat                 MAT;                    //!< typedef for `cv::Mat`
typedef cv::Scalar              SCALAR;                 //!< typedef for `cv::Scalar`.
typedef cv::Point2f             POINT2F;                //!< typedef for `cv::Point2f`.
typedef cv::Point2d             POINT2D;                //!< typedef for `cv::Point2d`.
typedef cv::Point2i             POINT2I;                //!< typedef for `cv::Point2i`.
typedef cv::Vec3b               VEC3B;                  //!< typedef for `cv::Vec3b`.
typedef cv::Size                CVSIZE;                 //!< typedef for `cv::Size`. This was selected as CVSIZE instead of SIZE to avoid conflicts with Microsoft SIZE
typedef cv::Rect                CVRECT;                 //!< typedef for `cv::Rect`. This was selected as CVRECT instead of RECT to avoid conflicts with Microsoft RECT
template<class T> using POINT2T = cv::Point_<T>;        //!< typedef for `cv::Point_<T>`.
#endif*/

// std c++
typedef std::string             STRING;                 //!< typedef for `std::string`
typedef std::wstring            WSTRING;                //!< typedef for `std::wstring`
typedef std::ostringstream      OSTRINGSTREAM;          //!< typedef for `std::ostringstream`
typedef std::ifstream           IFSTREAM;               //!< typedef for `std::ifstream`
typedef std::ofstream           OFSTREAM;               //!< typedef for `std::ofstream`
typedef std::stringstream       STRINGSTREAM;           //!< typedef for `std::stringstream`
typedef std::exception          EXCEPTION;              //!< typedef for `std::exception`
typedef std::runtime_error      EXCEPTION_RUNTIME;      //!< typedef for `std::runtime_error`
typedef std::mutex              MUTEX;                  //!< typedef for `std::mutex`
typedef std::thread             THREAD;                 //!< typedef for `std::thread`


// templated aliases
template<class T, class A = std::allocator<T>>                            using VECTOR            = std::vector<T, A>;                //!< typedef for `std::vector<T, A>`
template<class L, class R>                                                using PAIR              = std::pair<L, R>;                  //!< typedef for `std::pair<L, R>`
template<class L, class R>                                                using MAP               = std::map<L, R>;                   //!< typedef for `std::map<L, R>`
template<class L, class R>                                                using UMAP              = std::unordered_map<L, R>;         //!< typedef for `std::unordered_map<L, R>`
template<typename T, typename... Args>                                    using TUPLE             = std::tuple<T, Args...>;           //!< typedef for `std::tuple<T, Args...>`
template<typename T, typename D = std::default_delete<T>>                 using UNIQUE_POINTER    = std::unique_ptr<T, D>;            //!< typedef for `std::unique_ptr<T, D>`
template<typename T>                                                      using SHARED_POINTER    = std::shared_ptr<T>;               //!< typedef for `std::shared_ptr<T>`
template<typename T>                                                      using WEAK_POINTER      = std::weak_ptr<T>;                 //!< typedef for `std::weak_ptr<T>`
template<typename T>                                                      using ILIST             = std::initializer_list<T>;         //!< typedef for `std::initializer_list<T>`
template<typename T, class A = std::allocator<T>>                         using LIST              = std::list<T, A>;                  //!< typedef for `std::list<T, A>`
template<typename T, class C = std::less<T>, class A = std::allocator<T>> using SET               = std::set<T, C, A>;                //!< typedef for `std::set<T, C, A>`
template<typename T, class C = std::less<T>, class A = std::allocator<T>> using MULTISET          = std::multiset<T, C, A>;           //!< typedef for `std::multiset<T, C, A>`
template<typename T>                                                      using ATOMIC            = std::atomic<T>;                   //!< typedef for `std::atomic<T>`
template<typename T>                                                      using FUTURE            = std::future<T>;                   //!< typedef for `std::future<T>`
template<typename T>                                                      using SHARED_FUTURE     = std::shared_future<T>;            //!< typedef for `std::shared_future<T>`
template<typename T>                                                      using PROMISE           = std::promise<T>;                  //!< typedef for `std::promise<T>`

#endif // _HEADER_GAURD_ENGINEXG3_TYPES_HPP