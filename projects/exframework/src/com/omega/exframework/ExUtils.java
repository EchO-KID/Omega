package com.omega.exframework;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.Map;
import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import android.widget.Toast;

public class ExUtils {
	
	   //! baidu 
		//////////////////////////////////////////////////////////////////////////////////////////
		
		
		public static void exPlatformLogin()
		{
			ExFramework.getInstance().getMainActivity().runOnUiThread(new Runnable()
			{
				@Override
				public void run() {
					System.out.println("android: exPlatformLogin.");
					// TODO Auto-generated method stub
					
				}
			});
		}
		
		public static void exPlatformAccount(){
			ExFramework.getInstance().getMainActivity().runOnUiThread(new Runnable()
			{
				@Override
				public void run() {
					System.out.println("android: exPlatformAccount.");
					// TODO Auto-generated method stub
					
				}
			});
		}

		public static void exPlatformLogout()
		{
			System.out.println("android: exPlatformLogout!");
			ExFramework.getInstance().getMainActivity().runOnUiThread(new Runnable()    
	        {    
	            public void run()    
	            {    
//	            	BDGameSDK.logout();
//	            	BDGameSDK.closeFloatView(fygame.getActivity());//关闭悬浮窗
//	            	fygame.postExPlatformLogout();
	            }    
	      
	        });   
		}	
		
		public static void exPlatformPay(final String strPrice, final String strOrderId, final String strPropName, final String strProductId, final String strPlayerId, final String strNickName, final String strNotifyURL)
		{
			ExFramework.getInstance().getMainActivity().runOnUiThread(new Runnable()
			{
				@Override
				public void run()
				{
					
						
					
				}
			});
		}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	
	public static String getMetaDataString(String key)
	{
		String result = new String();
		try {
			
			Activity instance = ExFramework.getInstance().getMainActivity();
			ApplicationInfo appInfo = instance.getPackageManager().getApplicationInfo(instance.getPackageName(), PackageManager.GET_META_DATA);
			if (appInfo.metaData != null) 
				result = String.valueOf(appInfo.metaData.get(key));		
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			System.out.println("android getMetaDataString fail:" + key);
			e.printStackTrace();
		}		 
		//! System.out.println("android: key: " + key + "value: " + result);
		return result;
		
	}
	
	public static int getMetaDataInt(String key)
	{
		int result = -1;
		try {
			Activity instance = ExFramework.getInstance().getMainActivity();
			ApplicationInfo appInfo = instance.getPackageManager().getApplicationInfo(instance.getPackageName(), PackageManager.GET_META_DATA);
			if (appInfo.metaData != null) 
				result = appInfo.metaData.getInt(key);
				
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			System.out.println("android getMetaDataString fail:" + key);
			e.printStackTrace();
		}		 
		//! System.out.println("android: key: " + key + "value: " + result);
		return result;
	}
	 
	public static boolean sdkProvideExitTips(){
		return ExFramework.getInstance().provideExitTips();
	}
	
	public static void exitTips(){
								ExFramework.getInstance().getMainActivity().finish();
								finishAllActivity();
		                        android.os.Process.killProcess(android.os.Process.myPid());
	}
	
	public static void finishAllActivity()
    {
        try
        {
            Class localClass = Class.forName("android.app.ActivityThread");
            Method localMethod =
                localClass.getMethod("currentActivityThread", new Class[] {});
            Object localObject1 =
                localMethod.invoke(localClass, new Object[] {});
            Field localField1 = localClass.getDeclaredField("mActivities");
            localField1.setAccessible(true);
            
            Map localHashMap = null;
            if (localHashMap == null)
            {
                localHashMap = (Map) localField1.get(localObject1);
            }
            Iterator localIterator = localHashMap.entrySet().iterator();
            while (localIterator.hasNext())
            {
                Map.Entry localEntry = (Map.Entry) localIterator.next();
                Object localObject2 = localEntry.getValue();
                Class localObject3 = null;
                Class[] arrayOfClass = localClass.getDeclaredClasses();
                for (Class localObject5 : arrayOfClass)
                {
                    if ((!localObject5.getSimpleName().equals("ActivityRecord"))
                        && (!localObject5.getSimpleName()
                            .equals("ActivityClientRecord")))
                    {
                        continue;
                    }
                    localObject3 = localObject5;
                    break;
                }
                
                Object paused = localObject3.getDeclaredField("paused");
                ((Field) paused).setAccessible(true);
                boolean bool1 =
                    ((Boolean) ((Field) paused).get(localObject2)).booleanValue();
                Field localField2 = localObject3.getDeclaredField("stopped");
                localField2.setAccessible(true);
                boolean bool2 =
                    ((Boolean) localField2.get(localObject2)).booleanValue();
                if ((bool1) || (bool2))
                {
                    Field localField3 =
                        localObject3.getDeclaredField("activity");
                    localField3.setAccessible(true);
                    Activity localActivity =
                        (Activity) localField3.get(localObject2);
                    localActivity.finish();
                }
            }
        }
        catch (SecurityException localSecurityException)
        {
            localSecurityException.printStackTrace();
        }
        catch (NoSuchFieldException localNoSuchFieldException)
        {
            localNoSuchFieldException.printStackTrace();
        }
        catch (IllegalArgumentException localIllegalArgumentException)
        {
            localIllegalArgumentException.printStackTrace();
        }
        catch (IllegalAccessException localIllegalAccessException)
        {
            localIllegalAccessException.printStackTrace();
        }
        catch (NoSuchMethodException localNoSuchMethodException)
        {
            localNoSuchMethodException.printStackTrace();
        }
        catch (ClassNotFoundException localClassNotFoundException)
        {
            localClassNotFoundException.printStackTrace();
        }
        catch (InvocationTargetException localInvocationTargetException)
        {
            localInvocationTargetException.printStackTrace();
        }
    }

	public static native void setExPlatformProperty(String key, String value);
	public static native void initExPlatformType(String strOpt);    //! 参照ExPlatformType.h
	public static native void postExPlatformLoginSuccess(String sessionId, String uid,String nickName);
	public static native void postExPlatformLogout();
	public static native void postExPlatformInitSDKResult(int code);
	public static native void postExPlatformPayResult(String orderId, String payAmount, int code);
}
