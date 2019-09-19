#if 0
	/**  在onActivityResult中获取图片  db0 */
	public static native void onCamareOk(Bitmap bmp);//db0 
	public static native void onGpsOk(double latitude,double longitude);//db0 

	@Override protected void
		onActivityResult(int requestCode, int resultCode, Intent data)
		{
			Log.v(TAG,"sdl ---------onActivityResult------------,"+resultCode+"," + RESULT_OK+","+requestCode); 
			Bundle bundle = null;
			Bitmap bmp = null;
			switch(requestCode){
				case 1://get Camera bmp// see startActivityForResult
					if (resultCode == RESULT_OK)
					{ 
						bundle = data.getExtras();
						bmp = (Bitmap) bundle.get("data");
						if(bmp != null){ 
							//if(bmp != null){ img.setImageBitmap(bmp); }
							Log.v(TAG,"sdl bmp width:" + bmp.getWidth() + ", height:" + bmp.getHeight()); 
							Test.onCamareOk(bmp);
						}else{
							Log.v(TAG,"sdl bmp is null"); 
						}
					}
					break;
				case 2:// Video Capture
					if (resultCode == RESULT_OK)
					{ 
						bundle = data.getExtras();
						/*
						   vid = (Bitmap) bundle.get("data");
						   if(vid != null){ 
						   Toast.makeText(this, "Video saved to:\n" + data.getData()",  
						   Toast.LENGTH_LONG).show(); 

						   Test.onCamareOk(vid);
						   }else{
						   Log.v(TAG,"sdl vid is null"); 
						   }
						   */
					}
					break;
				case 3://pick bmp from album
					if (resultCode == RESULT_OK)
					{ 
						bundle = data.getExtras();
						bmp = (Bitmap) bundle.get("data");
						if(bmp != null){ 
							//if(bmp != null){ img.setImageBitmap(bmp); }
							Log.v(TAG,"sdl bmp width:" + bmp.getWidth() + ", height:" + bmp.getHeight()); 
							Test.onCamareOk(bmp);
						}else{
							Log.v(TAG,"sdl bmp is null"); 
						}
					}
					break;
				default:
					break;
			}
		}

	private Vibrator vibrator=null; 
	public void startVibrate(int i){
		Log.v(TAG,"sdl vibrate!"+i); 
		try {
			if(vibrator==null)
				vibrator = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);  
			long [] pattern = {100,400,100,400};   // 停止 开启 停止 开启   
			vibrator.vibrate(pattern,i);           //重复i次上面的pattern 如果只想震动一次，index设为-1   
		} catch (ActivityNotFoundException e) {
			e.printStackTrace();
		}
	}
	public void stopVibrate(){  
		try {
			if(vibrator!=null)
				vibrator.cancel();  
		} catch (ActivityNotFoundException e) {
			e.printStackTrace();
		}
	}  

	/**   启动Camera  db0*/
	public void startCam(){
		try {
			Intent cameraIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
			startActivityForResult(cameraIntent, 1);
		} catch (ActivityNotFoundException e) {
			e.printStackTrace();
		}
		Log.v(TAG,"sdl --------startCam-------------"); 
	}
	/**   启动Camera 录制 db0*/
	public void startVid(){
		Intent intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
		//fileUri = getOutputMediaFileUri(MEDIA_TYPE_VIDEO);  // create a file to save the video
		//intent.putExtra(MediaStore.EXTRA_OUTPUT, fileUri);  // set the image file name
		//intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 1); // set the video image quality to high
		// start the Video Capture Intent
		startActivityForResult(intent, 2);
	}
	//是打开系统的相册 db0
	public void openAlbum(){
		Intent albumIntent = new Intent(Intent.ACTION_PICK, null);
		albumIntent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
		startActivityForResult(albumIntent, 3);
		Log.v(TAG,"sdl --------openAlbum-------------"); 
	}
	private Toast toast=null;
	public void showToast(String s){
		Log.v(TAG,"sdl --------showToast-------------"+s); 
		if(toast!=null && toast.getView()!=null){
			try {
				//toast.cancel();
				//toast.setView(mTextEdit);
				toast.setDuration(Toast.LENGTH_SHORT);
				toast.setText(s);
				toast.show();
			} catch (Exception e) {
				Log.v(TAG,"sdl --------showToast Error!-------------"+e.toString()); 
			}
		}
	}
	private LocationManager locManager;

	//打开gps db0
	public void startGps() {
		// 创建LocationManager对象
		locManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE); 
		// 从GPS获取最近的最近的定位信息
		Location location = locManager.getLastKnownLocation(
				LocationManager.GPS_PROVIDER);
		// 使用location根据EditText的显示
		updateGps(location);
		// 设置每1秒获取一次GPS的定位信息
		locManager.requestLocationUpdates(LocationManager.GPS_PROVIDER 
				, 1000, 8, new LocationListener()
				{
					@Override
						public void onLocationChanged(Location location)
						{
							// 当GPS定位信息发生改变时，更新位置
							updateGps(location);
						}

					@Override
						public void onProviderDisabled(String provider)
						{
							updateGps(null);				
						}

					@Override
						public void onProviderEnabled(String provider)
						{
							// 当GPS LocationProvider可用时，更新位置
							updateGps(locManager.getLastKnownLocation(provider));				
						}

					@Override
						public void onStatusChanged(String provider, int status,
								Bundle extras)
						{
						}
				}); 

	}

	private void updateGps(Location newLocation)
	{
		if (newLocation != null)
		{
			StringBuilder sb = new StringBuilder();
			sb.append("实时的位置信息：\n");
			sb.append("经度：");
			sb.append(newLocation.getLongitude());
			sb.append("\n纬度：");
			sb.append(newLocation.getLatitude());
			sb.append("\n高度：");
			sb.append(newLocation.getAltitude());
			sb.append("\n速度：");
			sb.append(newLocation.getSpeed());
			sb.append("\n方向：");
			sb.append(newLocation.getBearing());			
			//Log.i(TAG, sb); 
			showToast(sb.toString());
			Test.onGpsOk(newLocation.getLatitude(),newLocation.getLongitude());
		}
		else
		{ 
			//Test.onGpsOk(newLocation.getLatitude(),newLocation.getLongitude());
		}
	}


#endif
#ifdef __ANDROID__

#include <android/bitmap.h>
#include "SDL.h"
#include "sprite.h"
#include <jni.h>

int startCam()
{
	// retrieve the JNI environment.
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

	// retrieve the Java instance of the SDLActivity
	jobject activity = (jobject)SDL_AndroidGetActivity();

	// find the Java class of the activity. It should be SDLActivity or a subclass of it.
	//jclass clazz(env->GetObjectClass(activity));
	jclass clazz = (*env)->GetObjectClass(env, activity);

	// find the identifier of the method to call
	jmethodID method_id = (*env)->GetMethodID(env,clazz, "startCam", "()V");
	//jmethodID method_id = (*env)->GetMethodID(env,clazz, "openAlbum", "()V");

	// effectively call the Java method
	(*env)->CallVoidMethod(env,activity, method_id);

	// clean up the local references.
	(*env)->DeleteLocalRef(env,activity);
	(*env)->DeleteLocalRef(env,clazz);
	return 1;
}

/* called by java onCamareOk */
JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_onCamareOk(JNIEnv *env, jclass clazz, jobject zBitmap) {  
	SDL_Log("--------------------------------------------------\n");  
	JNIEnv J = *env;  

	if (zBitmap == NULL) {  
		SDL_Log("bitmap is null\n");  
		return;  
	}  

	// Get bitmap info  
	AndroidBitmapInfo info;  
	memset(&info, 0, sizeof(info));  
	AndroidBitmap_getInfo(env, zBitmap, &info);  
	// Check format, only RGB565 & RGBA are supported  
	if (info.width <= 0 || info.height <= 0   
			|| (info.format == ANDROID_BITMAP_FORMAT_NONE)
	   ) {  
		SDL_Log("invalid bitmap\n");  
		J->ThrowNew(env, J->FindClass(env, "java/io/IOException"), "invalid bitmap");  
		return;  
	}  

	// Lock the bitmap to get the buffer  
	void * pixels = NULL;  
	int res = AndroidBitmap_lockPixels(env, zBitmap, &pixels);  
	if (pixels == NULL) {  
		SDL_Log("fail to lock bitmap: %d\n", res);  
		J->ThrowNew(env, J->FindClass(env, "java/io/IOException"), "fail to open bitmap");  
		return;
	}  

	Uint32 rmask, gmask, bmask, amask=0;
	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
	SDL_Log("Effect: %dx%d, %d\n", info.width, info.height, info.format);  
	int depth=0;
	switch(info.format)
	{
		case ANDROID_BITMAP_FORMAT_RGBA_8888:
			depth=32;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xff000000;
			gmask = 0x00ff0000;
			bmask = 0x0000ff00;
			amask = 0x000000ff;
#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = 0xff000000;
#endif
			break;
		case ANDROID_BITMAP_FORMAT_RGB_565:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0x1f<<11;
			gmask = 0x3f<<5;
			bmask = 0x1f;
#else
			bmask = 0x1f<<11;
			gmask = 0x3f<<5;
			rmask = 0x1f;
#endif
			depth=16;
			break;
		case ANDROID_BITMAP_FORMAT_RGBA_4444:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xf000;
			gmask = 0x0f00;
			bmask = 0x00f0;
			amask = 0x000f;
#else
			rmask = 0x000f;
			gmask = 0x00f0;
			bmask = 0x0f00;
			amask = 0xf000;
#endif
			depth=16;
			break;
		case ANDROID_BITMAP_FORMAT_A_8:
			break;
		case ANDROID_BITMAP_FORMAT_NONE:
			break;
	}
	SDL_Surface * surface = SDL_CreateRGBSurfaceFrom(pixels,
			info.width,
			info.height,
			depth,
			info.stride,
			rmask,
			gmask,
			bmask,
			amask
			);

	SDL_Surface * image = Surface_new(info.width,info.height);
	SDL_BlitSurface(surface, NULL, image, NULL);
	Sprite * sprite = Sprite_getChildByIndex(stage->sprite,0);
	Sprite_setSurface(sprite,image);
	image->refcount--;
	sprite->w = info.width;
	sprite->h = info.height;
	SDL_FreeSurface(surface);

	AndroidBitmap_unlockPixels(env, zBitmap);  
}  

#endif
