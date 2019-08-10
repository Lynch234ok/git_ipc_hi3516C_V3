
package
{
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.events.NetStatusEvent;
	import flash.events.TimerEvent;
	import flash.media.SoundTransform;
	import flash.media.Video;
	import flash.net.FileReference;
	import flash.net.NetConnection;
	import flash.net.NetStream;
	import flash.utils.Timer;
	
	public class JaVideo extends Video
	{	
		private var stream_url:String = ""; // the video stream url
		private var stream_name:String = ""; // the video stream name
		private var stream_client:Object = new Object();
		
		private var nc:NetConnection = null;
		public var ns:NetStream = null;
		
		public var bytesSpeed:uint = 0;
		public var bytesLoaded:uint = 0;
		public var freezing:uint = 0;
		
		private var timer:Timer = null;
		private var is_flv:Boolean = false;
		
		public function JaVideo(width:int=320, height:int=240)
		{
			super(width, height);
//			super.smoothing = true;
			super.smoothing = false;
		}
		
		private function timerCounter(event:TimerEvent):void
		{
			var bytes_loaded:uint = 0;
			
			try{
				if(!is_flv){
					bytes_loaded = this.ns.info.byteCount;
				}else{
					// flv streaming
					bytes_loaded = this.ns.bytesLoaded;
				}
			}catch(e:Error){
				trace("What!!");
				Reconnect();
				return;
			}
			
			const bytes_diff:uint = bytes_loaded - this.bytesLoaded;
			if(bytes_diff){
				this.freezing = 0;
			}else{
				// check freezing
				trace("current FPS = " + ns.currentFPS);
				if(0 == ns.currentFPS){
					this.freezing++;
					trace("video is freezing! " + this.freezing);
					if(this.freezing >= 5){
						// try to reconnect
						Reconnect();
					}
				}else{
					this.freezing = 0;
				}
			}
//			trace("Bytes " + bytes_loaded);
			this.bytesSpeed = bytes_diff;
			this.bytesLoaded = bytes_loaded; // update
//			trace("speed: " + this.bytesSpeed + " total: " + this.bytesLoaded);
		}
		
		private function streamPlay():void  
		{
			// create a net stream
			ns = new NetStream(nc);  
			ns.addEventListener(NetStatusEvent.NET_STATUS, netStatusListener);
			ns.backBufferTime = 0;
			ns.bufferTime = 0.2;
			ns.bufferTimeMax = 0;
			ns.client = stream_client;
			ns.play(stream_name);
			super.attachNetStream(ns);
			
			// start timer to stats
			timer = new Timer(1000);
			timer.start();
			timer.addEventListener(TimerEvent.TIMER, timerCounter);
		}
		
		private function netStatusListener(evt:NetStatusEvent):void
		{
			try{
				switch(evt.info.code)
				{
					case "NetConnection.Connect.Success":  
					{
						trace("Connect Success"); 
						streamPlay();
						
						break;  
					}
					// flv
					case "NetStream.Buffer.Full":
					{
						ns.bufferTime = 5; // 稍微增大缓冲以使视频更流畅
						break;
					}
					
					case "NetStream.Buffer.Empty":
					{
						ns.bufferTime = 0.5; // 设置比较小的缓冲以便尽快可以开始播放
						break;
					}
						
					case "NetConnection.Connect.Failed":
					{
						trace("RTMP Connect Failed");
						this.ConnectRTMP(this.stream_url, this.stream_name);
						break;
					}
						
					default:
					{
						trace(evt.info.code); 
					}
				}
			}catch(err:Error){
				trace("what?>");
			}
		}
		
		public function ConnectRTMP(url:String, stream_name:String): void
		{
			// mark down the stream url / name
			this.stream_url = url;
			this.stream_name = stream_name;
			
			nc = new NetConnection();
			// connection to the RTMP server first and listen its responsing
			nc.connect(this.stream_url);
			nc.addEventListener(NetStatusEvent.NET_STATUS, netStatusListener);
			
			// clear bytes conter
			bytesSpeed = 0;
			bytesLoaded = 0;
			freezing = 0;
			
		}
		
		public function ConnectFLV(url:String): void
		{
			is_flv = true; // the flv flag
			// mark down the stream name
			this.stream_name = url;
			
			nc = new NetConnection();
			nc.addEventListener(NetStatusEvent.NET_STATUS, netStatusListener);
			// connection to the FLV need to connect a url
			nc.connect(null); // the add event and connect ordering is different to rtmp mode

			// clear bytes conter
			bytesSpeed = 0;
			bytesLoaded = 0;
			freezing = 0;
		}
		
		public function Reconnect():void
		{
			if("" != this.stream_name){
				this.Disconnect();
				if(!is_flv){
					trace("Reconnect RTMP " + stream_url + stream_name);
					this.ConnectRTMP(this.stream_url, this.stream_name);
				}else{
					trace("Reconnect FLV " + stream_name);
					this.ConnectFLV(this.stream_name);
				}
			}
		}
		
		public function Disconnect():void
		{
			// stop the timer first
			if(timer){
				timer.stop();
				timer = null;
			}
			// close connection
			if(ns){
				ns.removeEventListener(NetStatusEvent.NET_STATUS, netStatusListener);
				ns.close(); // close netstream firstly
				ns = null;
			}
			if(nc){
				nc.removeEventListener(NetStatusEvent.NET_STATUS, netStatusListener);
				nc.close();
				nc = null;
			}
			clear();
		}
		
		private var sound_mute:Boolean = true;
		public function SetMute(mute:Boolean):void
		{
			sound_mute = mute;
			if(null != ns){
				try{
					ns.soundTransform = null;
					ns.soundTransform = new SoundTransform(sound_mute ? 0 : 1);
				}catch(e:Error){
					trace("对象无效。这可能是由于 NetConnection 失败造成的。")
				}
			}
		}
	}
}
