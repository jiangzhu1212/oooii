/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_libjpegturbo_turbojpeg_TJCompressor */

#ifndef _Included_org_libjpegturbo_turbojpeg_TJCompressor
#define _Included_org_libjpegturbo_turbojpeg_TJCompressor
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_init
  (JNIEnv *, jobject);

/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_destroy
  (JNIEnv *, jobject);

/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    compress
 * Signature: ([BIIII[BIII)I
 */
JNIEXPORT jint JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_compress___3BIIII_3BIII
  (JNIEnv *, jobject, jbyteArray, jint, jint, jint, jint, jbyteArray, jint, jint, jint);

/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    compress
 * Signature: ([IIIII[BIII)I
 */
JNIEXPORT jint JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_compress___3IIIII_3BIII
  (JNIEnv *, jobject, jintArray, jint, jint, jint, jint, jbyteArray, jint, jint, jint);

/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    encodeYUV
 * Signature: ([BIIII[BII)V
 */
JNIEXPORT void JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_encodeYUV___3BIIII_3BII
  (JNIEnv *, jobject, jbyteArray, jint, jint, jint, jint, jbyteArray, jint, jint);

/*
 * Class:     org_libjpegturbo_turbojpeg_TJCompressor
 * Method:    encodeYUV
 * Signature: ([IIIII[BII)V
 */
JNIEXPORT void JNICALL Java_org_libjpegturbo_turbojpeg_TJCompressor_encodeYUV___3IIIII_3BII
  (JNIEnv *, jobject, jintArray, jint, jint, jint, jint, jbyteArray, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
