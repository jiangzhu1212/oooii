// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oCompute_cdf97_h
#define oCompute_cdf97_h

namespace ouro {

// Forward biorthogonal CDF 9/7 wavelet transform. Transforms in place.
void cdf97fwd(float* _pValues, size_t _NumValues);

// Inverse biorthogonal CDF 9/7 wavelet transform. Transforms in place.
void cdf97inv(float* _pValues, size_t _NumValues);

} // namespace ouro

#endif
