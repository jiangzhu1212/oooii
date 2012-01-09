// $(header)
#include <oBasis/oEqual.h>
#include <oBasis/oAssert.h>

bool oEqual(const float& A, const float& B, int maxUlps)
{
	/** <citation
		usage="Implementation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="changed assert macro"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	oASSERT(maxUlps > 0 && maxUlps < 4 * 1024 * 1024, "");
	int aInt = *(int*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&B;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	int intDiff = abs(aInt - bInt);
	if (intDiff <= maxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}

bool oEqual(const double& A, const double& B, int maxUlps)
{
	typedef long long intT;

	/** <citation
		usage="Adaptation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="changed assert macro and types to accommodate doubles. Express abs() explicitly because VC9 doesn't seem to have a long long implementation"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	oASSERT(maxUlps > 0 && maxUlps < 4 * 1024 * 1024, "");
	intT aInt = *(intT*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x8000000000000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	intT bInt = *(intT*)&B;
	if (bInt < 0)
		bInt = 0x8000000000000000 - bInt;
	intT diff = aInt - bInt;
	intT intDiff = diff >= 0 ? diff : -diff; // abs() seems only implemented for int/long in VC9
	if (intDiff <= maxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}