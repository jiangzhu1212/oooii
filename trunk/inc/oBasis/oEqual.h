// $(header)
// Approximate equality for float/double types. This uses absolute bit 
// differences rather than epsilon or some very small float value because eps
// usage isn't valid across the entire range of floating precision. With this
// oEqual API it can be guaranteed that the statement "equal within N minimal 
// bits of difference" will be true.
#pragma once
#ifndef oEqual_h
#define oEqual_h

// ulps = "units of last place". Number of float-point steps of error. At various
// sizes, 1 bit of difference in the floating point number might mean large or
// small deltas, so just doing an epsilon difference is not valid across the
// entire spectrum of floating point representations. With ULPS, you specify the
// maximum number of floating-point steps, not absolute (fixed) value that some-
// thing should differ by, so it scales across all of float's range.
#define oDEFAULT_ULPS 5

template<typename T> inline bool oEqual(const T& A, const T& B, int maxUlps = oDEFAULT_ULPS) { return A == B; }

bool oEqual(const float& A, const float& B, int maxUlps = oDEFAULT_ULPS);
bool oEqual(const double& A, const double& B, int maxUlps = oDEFAULT_ULPS);

#endif
