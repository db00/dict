/*
 * =====================================================================================
 *
 *       Filename:  map.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/09/2016 09:55:16 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  db0@qq.com (db0), db0@qq.com
 *        Company:  huainen
 *

 dumpsys location
dumpsys sensorservice
http://www.google.cn/maps/@26.8962415,112.6115503,19z
am start -a android.intent.action.VIEW geo:26.8962415,112.6115503?q=地址名
 * =====================================================================================
 */

#ifdef __ANDROID__

#include <android/bitmap.h>
#include "SDL.h"
#include "sprite.h"
#include <jni.h>

int startGps()
{
	// retrieve the JNI environment.
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

	// retrieve the Java instance of the SDLActivity
	jobject activity = (jobject)SDL_AndroidGetActivity();

	// find the Java class of the activity. It should be SDLActivity or a subclass of it.
	//jclass clazz(env->GetObjectClass(activity));
	jclass clazz = (*env)->GetObjectClass(env, activity);

	// find the identifier of the method to call
	jmethodID method_id = (*env)->GetMethodID(env,clazz, "startGps", "()V");

	// effectively call the Java method
	(*env)->CallVoidMethod(env,activity, method_id);

	// clean up the local references.
	(*env)->DeleteLocalRef(env,activity);
	(*env)->DeleteLocalRef(env,clazz);
	return 1;
}

/* called by java onCamareOk */
JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onGpsOk(JNIEnv *env, jclass clazz,jdouble getLatitude,jdouble getLongitude) {  
	SDL_Log("--------------------------------------------------gps ok\n");  
	JNIEnv J = *env;  
	SDL_Log("gps:%lf,%lf\n",getLatitude,getLongitude);  
	//char * cmd = append_str(NULL,"am start -a android.intent.action.VIEW geo:%f,%f",getLatitude,getLongitude);
	//system(cmd);
	//free(cmd);
}  

#endif

