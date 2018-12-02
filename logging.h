/*
 * logging.h
 *
 *  Created on: Jan 1, 2015
 *      Author: Mike
 */

#ifndef LOGGING_H_
#define LOGGING_H_


//#define LOGGING
#ifdef LOGGING
#define NLOGZ(A) cout << A
#define NLOG(A) cout << __FUNCTION__ << ' ' << A << ' ' << __FILE__ << ":" << __LINE__ << endl
#elif 0
class sink_ {};
extern sink_ sink;
template <typename T>
sink_& operator<<(sink_& s, const T&) { return s; }
inline sink_& endl(sink_& s) { return s; }
#define NLOG(A) sink << A
#else
#define NLOG(A)
#endif



#endif /* LOGGING_H_ */
