﻿using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Lumino
{
    /// <summary>
    /// 初期設定を行います。
    /// </summary>
    public partial class Config
    {
    
    
        /// <summary>
        /// デバッグ用のログファイルの出力有無を設定します。(既定値:false)
        /// </summary>
        /// <param name="enabled">true:出力する / false:出力しない</param>
        public static void SetApplicationLogEnabled( bool enabled)
        {
            API.LNConfig_SetApplicationLogEnabled( enabled);
        
        }
        
        /// <summary>
        /// 標準入出力用のコンソールウィンドウを割り当てるかどうかを設定します。(既定値:false)
        /// </summary>
        /// <param name="enabled">true:割り当てる / false:割り当てない</param>
        public static void SetConsoleEnabled( bool enabled)
        {
            API.LNConfig_SetConsoleEnabled( enabled);
        
        }
        
        /// <summary>
        /// ファイルを開くときにアクセスする暗号化アーカイブを登録します。
        /// </summary>
        /// <param name="filePath">アーカイブファイルパス</param>
        /// <param name="password">アーカイブファイルを開くためのパスワード</param>
        public static void RegisterArchive( string filePath,  string password)
        {
            API.LNConfig_RegisterArchive( filePath,  password);
        
        }
        
        /// <summary>
        /// ファイルへのアクセス優先順位を設定します。
        /// </summary>
        /// <param name="priority">制限方法 (default:FileAccessPriority.DirectoryFirst)</param>
        public static void SetFileAccessPriority( FileAccessPriority priority)
        {
            API.LNConfig_SetFileAccessPriority( priority);
        
        }
        
        /// <summary>
        /// ユーザー定義のウィンドウハンドルを設定します。(既定値:NULL)
        /// </summary>
        /// <param name="windowHandle">ユーザー定義のウィンドウハンドル</param>
        public static void SetUserWindowHandle( IntPtr windowHandle)
        {
            API.LNConfig_SetUserWindowHandle( windowHandle);
        
        }
        
        /// <summary>
        /// サウンドオブジェクトのキャッシュサイズの設定
        /// </summary>
        /// <param name="objectCount">キャッシュできるサウンドオブジェクトの最大数 (既定値:32)</param>
        /// <param name="memorySize">サウンドオブジェクトのキャッシュが使用できる最大メモリサイズ (既定値:0)</param>
        /// <remarks>
        /// objectCount が 0 の場合、キャッシュを使用しません。
        /// 					memorySize が 0 の場合、メモリ使用量に制限を設けません。
        /// </remarks>
        public static void SetSoundCacheSize( int objectCount,  int memorySize)
        {
            API.LNConfig_SetSoundCacheSize( objectCount,  memorySize);
        
        }
        
        /// <summary>
        /// DirectMusic の初期化方法を設定します。(既定値:DirectMusicMode.NotUse)
        /// </summary>
        /// <param name="mode">DirectMusic の初期化方法</param>
        /// <remarks>
        /// DirectMusic の初期化には比較的時間がかかります。
        /// 					これを回避するために初期化専用のスレッドで初期化を行うことが出来ます。
        /// </remarks>
        public static void SetDirectMusicMode( DirectMusicMode mode)
        {
            API.LNConfig_SetDirectMusicMode( mode);
        
        }
        
        /// <summary>
        /// DirectMusic のリバーブエフェクトの強さを設定します。(既定値:0.75)
        /// </summary>
        /// <param name="level">リバーブの強さ (0.0 ～ 1.0)</param>
        public static void SetDirectMusicReverbLevel( float level)
        {
            API.LNConfig_SetDirectMusicReverbLevel( level);
        
        }
        
    
    };
    
    /// <summary>
    /// ライブラリ全体の初期化や更新等、包括的な処理を行うクラスです。
    /// </summary>
    public partial class Application
    {
    
    
        /// <summary>
        /// ライブラリを初期化します。音声機能のみを使用する場合に呼び出します。
        /// </summary>
        public static void InitializeAudio()
        {
            InternalManager.Initialize();
            var result = API.LNApplication_InitializeAudio();
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// ライブラリの終了処理を行います。
        /// </summary>
        public static void Terminate()
        {
            InternalManager.Terminate();
            API.LNApplication_Terminate();
        
        }
        
    
    };
    
    /// <summary>
    /// バージョン情報です。
    /// </summary>
    public partial class Version
    {
    
    
        /// <summary>
        /// メジャーバージョンを取得します。
        /// </summary>
        public static int GetMajor()
        {
            var outMajor = new int();
            API.LNVersion_GetMajor(out outMajor);
            return outMajor;
        
        }
        
        /// <summary>
        /// マイナーバージョンを取得します。
        /// </summary>
        public static int GetMinor()
        {
            var outMinor = new int();
            API.LNVersion_GetMinor(out outMinor);
            return outMinor;
        
        }
        
        /// <summary>
        /// リビジョンバージョンを取得します。
        /// </summary>
        public static int GetRevision()
        {
            var outRevision = new int();
            API.LNVersion_GetRevision(out outRevision);
            return outRevision;
        
        }
        
        /// <summary>
        /// バージョン文字列を取得します。
        /// </summary>
        public static string GetString()
        {
            IntPtr strPtr;
            API.LNVersion_GetString(out strPtr);
            int len;
            API.LCSInternal_GetIntPtrStringLength(strPtr, out len);
            var sb = new StringBuilder(len + 1);
            API.LCSInternal_GetIntPtrString(strPtr, sb);
            return sb.ToString();
        }
        
        /// <summary>
        /// 指定したバージョン番号と、ライブラリファイルのコンパイルバージョン番号を比較します。
        /// </summary>
        /// <param name="major">メジャーバージョン</param>
        /// <param name="minor">マイナーバージョン</param>
        /// <param name="revision">リビジョンバージョン</param>
        /// <remarks>
        /// 指定バージョン >= コンパイルバージョン である場合、true となります。
        /// </remarks>
        public static bool IsAtLeast( int major,  int minor,  int revision)
        {
            var outResult = new bool();
            API.LNVersion_IsAtLeast( major,  minor,  revision, out outResult);
            return outResult;
        
        }
        
    
    };
    
    /// <summary>
    /// ゲーム向け音声再生のユーティリティクラスです。
    /// </summary>
    public partial class GameAudio
    {
    
    
        /// <summary>
        /// BGM を演奏します。
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        /// <param name="fadeTime">フェードインにかける時間 (秒)</param>
        public static void PlayBGM( string filePath,  float volume = 1.0f,  float pitch = 1.0f,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_PlayBGM( filePath,  volume,  pitch,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータを使用して BGM を演奏します。
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        /// <param name="fadeTime">フェードインにかける時間 (秒)</param>
        public static void PlayBGMMem( byte[] data,  int dataSize,  float volume = 1.0f,  float pitch = 1.0f,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_PlayBGMMem( data,  dataSize,  volume,  pitch,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// BGM の演奏を停止します。
        /// </summary>
        /// <param name="fadeTime">フェードアウトにかける時間 (秒)</param>
        public static void StopBGM( double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_StopBGM( fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// BGS を演奏します。
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        /// <param name="fadeTime">フェードインにかける時間 (秒)</param>
        public static void PlayBGS( string filePath,  float volume = 1.0f,  float pitch = 1.0f,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_PlayBGS( filePath,  volume,  pitch,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータから BGS を演奏します。
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        /// <param name="fadeTime">フェードインにかける時間 (秒)</param>
        public static void PlayBGSMem( byte[] data,  int dataSize,  float volume = 1.0f,  float pitch = 1.0f,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_PlayBGSMem( data,  dataSize,  volume,  pitch,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// BGS の演奏を停止します。、
        /// </summary>
        /// <param name="fadeTime">フェードアウトにかける時間 (秒)</param>
        public static void StopBGS( double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_StopBGS( fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// ME を演奏します。
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlayME( string filePath,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlayME( filePath,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータから ME を演奏します。
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlayMEMem( byte[] data,  int dataSize,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlayMEMem( data,  dataSize,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// ME の演奏を停止します。
        /// </summary>
        public static void StopME()
        {
            var result = API.LNGameAudio_StopME();
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// SE を演奏します。
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySE( string filePath,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySE( filePath,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// SE を演奏します。(3D サウンド)
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="position">3D 空間上の座標</param>
        /// <param name="distance">減衰距離</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySE3D( string filePath,  Vector3 position,  float distance,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySE3D( filePath, ref position,  distance,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// SE を演奏します。(3D サウンド)
        /// </summary>
        /// <param name="filePath">ファイルパス</param>
        /// <param name="x">3D 空間上の X 座標</param>
        /// <param name="y">3D 空間上の Y 座標</param>
        /// <param name="z">3D 空間上の Z 座標</param>
        /// <param name="distance">減衰距離</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySE3D( string filePath,  float x,  float y,  float z,  float distance,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySE3DXYZ( filePath,  x,  y,  z,  distance,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声データから SE を演奏します。
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySEMem( byte[] data,  int dataSize,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySEMem( data,  dataSize,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータから SE を演奏します。 (3D サウンド)
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="position">3D 空間上の座標</param>
        /// <param name="distance">減衰距離</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySE3DMem( byte[] data,  int dataSize,  Vector3 position,  float distance,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySE3DMem( data,  dataSize, ref position,  distance,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータから SE を演奏します。(3D サウンド)
        /// </summary>
        /// <param name="data">メモリ上の音声ファイルデータ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        /// <param name="x">3D 空間上の X 座標</param>
        /// <param name="y">3D 空間上の Y 座標</param>
        /// <param name="z">3D 空間上の Z 座標</param>
        /// <param name="distance">減衰距離</param>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="pitch">ピッチ (0.5～2.0)</param>
        public static void PlaySE3DMem( byte[] data,  int dataSize,  float x,  float y,  float z,  float distance,  float volume = 1.0f,  float pitch = 1.0f)
        {
            var result = API.LNGameAudio_PlaySE3DMemXYZ( data,  dataSize,  x,  y,  z,  distance,  volume,  pitch);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// すべての SE の演奏を停止します。
        /// </summary>
        public static void StopSE()
        {
            var result = API.LNGameAudio_StopSE();
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// 再生中のBGMの音量を設定します。(フェードアウト中は無効)
        /// </summary>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="fadeTime">フェードアウトにかける時間 (秒)</param>
        public static void SetBGMVolume( float volume,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_SetBGMVolume( volume,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// 再生中のBGSの音量を設定します。(フェードアウト中は無効)
        /// </summary>
        /// <param name="volume">ボリューム (0.0～1.0)</param>
        /// <param name="fadeTime">フェードアウトにかける時間 (秒)</param>
        public static void SetBGSVolume( float volume,  double fadeTime = 0.0)
        {
            var result = API.LNGameAudio_SetBGSVolume( volume,  fadeTime);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
    
    };
    
    /// <summary>
    /// 3D音声のリスナーに関する情報を操作します。
    /// </summary>
    public partial class SoundListener
    {
    
        /// <summary>
        /// 3D音声のリスナーの位置
        /// </summary>
        public static Vector3 Position
        {
            set
            {
                var result = API.LNSoundListener_SetPosition(ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// 3D音声のリスナーの正面方向
        /// </summary>
        public static Vector3 Direction
        {
            set
            {
                var result = API.LNSoundListener_SetDirection(ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// 3D音声のリスナーの上方向(正面方向とは直交であること)
        /// </summary>
        public static Vector3 UpDirection
        {
            set
            {
                var result = API.LNSoundListener_SetUpDirection(ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// 3D音声のリスナーの速度
        /// </summary>
        public static Vector3 Velocity
        {
            set
            {
                var result = API.LNSoundListener_SetVelocity(ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
    
        /// <summary>
        /// 3D音声のリスナーの位置を設定します。
        /// </summary>
        /// <param name="x">3D 空間上の X 座標</param>
        /// <param name="y">3D 空間上の Y 座標</param>
        /// <param name="z">3D 空間上の Z 座標</param>
        public static void SetPositionXYZ( float x,  float y,  float z)
        {
            var result = API.LNSoundListener_SetPositionXYZ( x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// 3D音声のリスナーの正面方向を設定します。
        /// </summary>
        /// <param name="x">向きの X 成分</param>
        /// <param name="y">向きの Y 成分</param>
        /// <param name="z">向きの Z 成分</param>
        public static void SetDirection( float x,  float y,  float z)
        {
            var result = API.LNSoundListener_SetDirectionXYZ( x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// 3D音声のリスナーの上方向を設定します。(正面方向とは直交であること)
        /// </summary>
        /// <param name="x">向きの X 成分</param>
        /// <param name="y">向きの Y 成分</param>
        /// <param name="z">向きの Z 成分</param>
        public static void SetUpDirection( float x,  float y,  float z)
        {
            var result = API.LNSoundListener_SetUpDirectionXYZ( x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// 3D音声のリスナーの速度を設定します。
        /// </summary>
        /// <param name="x">速度の X 成分</param>
        /// <param name="y">速度の Y 成分</param>
        /// <param name="z">速度の Z 成分</param>
        public static void SetVelocity( float x,  float y,  float z)
        {
            var result = API.LNSoundListener_SetVelocityXYZ( x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
    
    };
    
    /// <summary>
    /// 音声データひとつ分を表し、再生などの操作を行うクラスです。
    /// </summary>
    public partial class Sound : RefObject
    {
    
        /// <summary>
        /// サウンドのボリューム
        /// </summary>
        public float Volume
        {
            get
            {
                var outVolume = new float();
                var result = API.LNSound_GetVolume( _handle, out outVolume);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outVolume;
            
            }
            
            set
            {
                var result = API.LNSound_SetVolume( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドのピッチ
        /// </summary>
        public float Pitch
        {
            get
            {
                var outPitch = new float();
                var result = API.LNSound_GetPitch( _handle, out outPitch);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outPitch;
            
            }
            
            set
            {
                var result = API.LNSound_SetPitch( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドのループ再生が有効であるか
        /// </summary>
        public bool IsLoopEnabled
        {
            get
            {
                var outEnabled = new bool();
                var result = API.LNSound_IsLoopEnabled( _handle, out outEnabled);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outEnabled;
            
            }
            
            set
            {
                var result = API.LNSound_SetLoopEnabled( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドが 3D 音源であるか
        /// </summary>
        public bool Is3DEnabled
        {
            get
            {
                var outEnabled = new bool();
                var result = API.LNSound_Is3DEnabled( _handle, out outEnabled);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outEnabled;
            
            }
            
            set
            {
                var result = API.LNSound_Set3DEnabled( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンド再生時の音声データの再生方法
        /// </summary>
        public SoundPlayingMode PlayingMode
        {
            get
            {
                var outMode = new SoundPlayingMode();
                var result = API.LNSound_GetPlayingMode( _handle, out outMode);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outMode;
            
            }
            
            set
            {
                var result = API.LNSound_SetPlayingMode( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドの再生状態
        /// </summary>
        public SoundPlayingState PlayingState
        {
            get
            {
                var outState = new SoundPlayingState();
                var result = API.LNSound_GetPlayingState( _handle, out outState);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outState;
            
            }
            
        }
        /// <summary>
        /// サウンドの再生したサンプル数
        /// </summary>
        public long PlayedSamples
        {
            get
            {
                var outSamples = new long();
                var result = API.LNSound_GetPlayedSamples( _handle, out outSamples);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outSamples;
            
            }
            
        }
        /// <summary>
        /// サウンドの音声データ全体のサンプル数
        /// </summary>
        public long TotalSamples
        {
            get
            {
                var outSamples = new long();
                var result = API.LNSound_GetTotalSamples( _handle, out outSamples);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outSamples;
            
            }
            
        }
        /// <summary>
        /// サウンドのサンプリングレート
        /// </summary>
        public int SamplingRate
        {
            get
            {
                var outRate = new int();
                var result = API.LNSound_GetSamplingRate( _handle, out outRate);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
                return outRate;
            
            }
            
        }
        /// <summary>
        /// サウンドの 3D 音源としての位置
        /// </summary>
        public Vector3 EmitterPosition
        {
            set
            {
                var result = API.LNSound_SetEmitterPosition( _handle, ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドの 3D 音源としての速度
        /// </summary>
        public Vector3 EmitterVelocity
        {
            set
            {
                var result = API.LNSound_SetEmitterVelocity( _handle, ref value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
        /// <summary>
        /// サウンドの 3D 音源の減衰距離 (聴こえなくなる距離) (default:100)
        /// </summary>
        public float EmitterMaxDistance
        {
            set
            {
                var result = API.LNSound_SetEmitterMaxDistance( _handle,  value);
                if (result != Result.OK) {
                    IntPtr errStr;
                    int errStrLen;
                    API.LNError_GetLastErrorMessage(out errStr);
                    API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                    var errBuf = new StringBuilder(errStrLen);
                    API.LCSInternal_GetIntPtrString(errStr, errBuf);
                    throw new LuminoException(result, errBuf.ToString());
                }
            
            }
            
        }
    
        internal Sound(_LNInternal i) : base(i) {}
        
        /// <summary>
        /// ファイルからサウンドオブジェクトを作成します。
        /// </summary>
        /// <param name="filePath">音声ファイルのパス</param>
        public  Sound( string filePath) : base(_LNInternal.InternalBlock)
        {
            IntPtr sound;
            var result = API.LNSound_Create( filePath, out sound);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
            InternalManager.RegisterWrapperObject(this, sound);
        
        }
        
        /// <summary>
        /// メモリ上の音声ファイルデータからサウンドオブジェクトを作成します。
        /// </summary>
        /// <param name="data">メモリ上の音声データへのポインタ</param>
        /// <param name="dataSize">データサイズ (バイト単位)</param>
        public  Sound( byte[] data,  int dataSize) : base(_LNInternal.InternalBlock)
        {
            IntPtr sound;
            var result = API.LNSound_CreateMem( data,  dataSize, out sound);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
            InternalManager.RegisterWrapperObject(this, sound);
        
        }
        
        /// <summary>
        /// サウンドのループ再生の範囲を設定します。
        /// </summary>
        /// <param name="begin">ループ領域の先頭位置 (サンプル数単位)</param>
        /// <param name="length">ループ領域長さ (サンプル数単位)</param>
        /// <remarks>
        /// begin と length に 0 を指定すると、全体をループ領域として設定します。
        /// </remarks>
        public void SetLoopRange( int begin,  int length)
        {
            var result = API.LNSound_SetLoopRange( _handle,  begin,  length);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドを再生します。
        /// </summary>
        public void Play()
        {
            var result = API.LNSound_Play( _handle);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドの再生を停止します。
        /// </summary>
        public void Stop()
        {
            var result = API.LNSound_Stop( _handle);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドの再生を一時停止します。
        /// </summary>
        public void Pause()
        {
            var result = API.LNSound_Pause( _handle);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドの再生を一時停止します。
        /// </summary>
        public void Resume()
        {
            var result = API.LNSound_Resume( _handle);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンド音量のフェード操作を行います。
        /// </summary>
        /// <param name="targetVolume">変更先の音量</param>
        /// <param name="time">フェードにかける時間 (秒)</param>
        /// <param name="behavior">フェード完了後の動作の指定</param>
        public void FadeVolume( float targetVolume,  double time,  SoundFadeBehavior behavior)
        {
            var result = API.LNSound_FadeVolume( _handle,  targetVolume,  time,  behavior);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドの 3D 音源としての位置を設定します。
        /// </summary>
        /// <param name="x">3D 空間上の X 座標</param>
        /// <param name="y">3D 空間上の Y 座標</param>
        /// <param name="z">3D 空間上の Z 座標</param>
        public void SetEmitterPosition( float x,  float y,  float z)
        {
            var result = API.LNSound_SetEmitterPositionXYZ( _handle,  x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
        /// <summary>
        /// サウンドの 3D 音源としての速度を設定します。
        /// </summary>
        /// <param name="x">速度の X 成分</param>
        /// <param name="y">速度の Y 成分</param>
        /// <param name="z">速度の Z 成分</param>
        public void SetEmitterVelocity( float x,  float y,  float z)
        {
            var result = API.LNSound_SetEmitterVelocityXYZ( _handle,  x,  y,  z);
            if (result != Result.OK) {
                IntPtr errStr;
                int errStrLen;
                API.LNError_GetLastErrorMessage(out errStr);
                API.LCSInternal_GetIntPtrStringLength(errStr, out errStrLen);
                var errBuf = new StringBuilder(errStrLen);
                API.LCSInternal_GetIntPtrString(errStr, errBuf);
                throw new LuminoException(result, errBuf.ToString());
            }
        
        }
        
    
    };
    
	

    


    internal class TypeInfo
    {
        public delegate RefObject ReferenceObjectFactory(IntPtr handle);

        public ReferenceObjectFactory Factory;

        private static List<TypeInfo> _typeInfos = new List<TypeInfo>();

        public static void Register()
        {

var _Sound = new TypeInfo(){ Factory = (handle) =>
    {
        var obj = new Sound(_LNInternal.InternalBlock);
        obj.SetHandle(handle);
        return obj;
    }
};
_typeInfos.Add(_Sound);
LNSound_SetBindingTypeInfo((IntPtr)(_typeInfos.Count - 1));

        }

        public static TypeInfo GetTypeInfoByHandle(IntPtr handle)
        {
            int index = (int)LNObject_GetTypeUserData(handle);
            return _typeInfos[index];
        }

        [DllImport(API.DLLName, CallingConvention = API.DefaultCallingConvention)]
        private static extern IntPtr LNObject_GetTypeUserData(IntPtr handle);
[DllImport(API.DLLName, CallingConvention = API.DefaultCallingConvention)]
private static extern void LNSound_SetBindingTypeInfo(IntPtr data);


    }

#if false
    /// <summary>
    /// オブジェクトのコレクション
    /// </summary>
    public class ObjectList<T> : ReferenceObject
        where T : ReferenceObject
    {
        private List<ReferenceObject> _list;
        
        internal ObjectList(_LNInternal i) {}

        internal override void SetHandle(IntPtr handle)
        {
            _list = new List<ReferenceObject>();
            _handle = handle;
            int count = Count;
            for (int i = 0; i < count; i++)
            {
                IntPtr item;
                API.LNObjectList_GetAt(_handle, i, out item);
                var t = TypeInfo.GetTypeInfoByHandle(item).Factory(item);
                t.SetHandle(item);
                _list.Add(t);
            }
        }
        
        public int Count
        {
            get
            {
                int count;
                API.LNObjectList_GetCount(_handle, out count);
                return count;
            }
        }
        
        public T this[int index]
        {
            get
            {
                SyncItems();
                return (T)_list[index];
            }
            set
            {
                SyncItems();
                _list[index] = value;
                API.LNObjectList_SetAt(_handle, index, value.Handle);
            }
        }
        
        public void Add(T item)
        {
            SyncItems();
            _list.Add(item);
            API.LNObjectList_Add(_handle, item.Handle);
        }
        
        public void Clear()
        {
            _list.Clear();
            API.LNObjectList_Clear(_handle);
        }
        
        public void Insert(int index, T item)
        {
            SyncItems();
            _list.Insert(index, item);
            API.LNObjectList_Insert(_handle, index, item.Handle);
        }
        
        public void Remove(T item)
        {
            SyncItems();
            _list.Remove(item);
            API.LNObjectList_Remove(_handle, item.Handle);
        }
        
        public void RemoveAll(T item)
        {
            SyncItems();
            _list.RemoveAll((i) => i == item);
            API.LNObjectList_RemoveAll(_handle, item.Handle);
        }
        
        public void RemoveAt(int index)
        {
            SyncItems();
            _list.RemoveAt(index);
            API.LNObjectList_RemoveAt(_handle, index);
        }

        private void SyncItems() 
        {
            int count = Count;
            if (_list.Count < count)
            {
                // 足りない分を詰める
                for (int i = 0; i < count - _list.Count; ++i)
		        {
                    _list.Add(null);
                }

                // リスト内容を同期する
                for (int i = 0; i < count; ++i)
		        {
                    IntPtr item;
                    API.LNObjectList_GetAt(_handle, i, out item);
			        if (_list[i] == null || _list[i].Handle != item)
			        {
                        var t = TypeInfo.GetTypeInfoByHandle(item).Factory(item);
                        t.SetHandle(item);
                        _list.Add(t);
			        }
		        }
            }
        }
    }
#endif

	

}
