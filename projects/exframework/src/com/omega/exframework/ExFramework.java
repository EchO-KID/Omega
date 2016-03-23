package com.omega.exframework;

import com.baidu.mtjstatsdk.StatSDKService;
import com.baidu.mtjstatsdk.game.BDGameSDK;

import android.app.Activity;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;


public class ExFramework {
	
	public static interface RunOnGLThread {
		
		public void runOnCocosThread(final Runnable pRunnable);
	}
	
	public static ExFramework getInstance(){
		if ( null == ms_instance ){
			ms_instance = new ExFramework();
		}
		return ms_instance;
	}
	protected static ExFramework ms_instance = null;
	
	RunOnGLThread getGLThread(){
		return glThread;
	}
	protected RunOnGLThread glThread = null;
	
	public Activity getMainActivity(){
		return mainActivity;
	}
	protected Activity mainActivity = null;
	
	//! 可以改成从firebase 获取
	protected String bdAppKey = "0e7d7ebbea";
	
	public void onMACreate(Activity activity, final RunOnGLThread glThread){
		this.mainActivity = activity;
		this.glThread     = glThread;
		System.out.println("android: onMACreate ");
		
		//! baidu
		PackageInfo pInfo = null;
		try {
			pInfo = mainActivity.getPackageManager().getPackageInfo(mainActivity.getPackageName(), 0);
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		String versionName = "1.0";
		if ( null!=pInfo )
			versionName = pInfo.versionName;
		
        BDGameSDK.setOn(mainActivity, BDGameSDK.EXCEPTION_LOG, bdAppKey); // 统计崩溃信息，必须优先其他接口调用
        BDGameSDK.initGame(mainActivity, bdAppKey);                       // 初始化游戏统计，必须优先其他接口调用，和setOn一起调用。
        StatSDKService.setAppChannel(mainActivity, "omega", true, bdAppKey); // 初始化app的渠道号，优先调用，和initGame以及setOn一起调用
        StatSDKService.setDebugOn(false, bdAppKey);              // 打开log调试，log的过滤标签位statsdk，可以看到发送的log
        StatSDKService.setAppVersionName(versionName, bdAppKey); // 设置APP的版本号，必须优先调用，与上面几个初始化接口一起调用。
        BDGameSDK.setAccount(mainActivity, StatSDKService.getCuid(mainActivity), bdAppKey); // 在游戏首页，需要添加游戏账户的id，如果游戏开始，没有设置游戏的ID，那么游戏数据无法统计到。可以
        StatSDKService.onEvent(mainActivity, "init_activity", StatSDKService.getCuid(mainActivity), 1, bdAppKey);
		
	}
	
	public void onMAStart(Activity activity){
		System.out.println("android: onMAStart ");
		
	
	}
	
	public void onMARestart(Activity activity){
		System.out.println("android onMARestart");
	}
	
	public void onMAPause(Activity activity){
		System.out.println("android: onMAPause ");
		
		StatSDKService.onPause(mainActivity, bdAppKey);
	}
	
	public void onMAResume(Activity activity){
		System.out.println("android: onMAResume ");
		
		StatSDKService.onResume(mainActivity, bdAppKey);
	}
	
	public void onMAStop(Activity activity){
		
		System.out.println("android: onMAStop ");
		
	}
	
	public void onMADestroy(Activity activity){
		System.out.println("android: onMADestroy ");
		
	}
	
	public void onMAFinish(Activity activity){
		// TODO Auto-generated method stub
		System.out.println("android: onMAFinish ");	
		
	}
	
	//! sdk 是否有自己的退出界面
	public boolean provideExitTips(){
		return false;
	}
	
	public void exitTips(){
		//! do nothing.
//		mainActivity.finish();
//		System.exit(0);
	}
}
