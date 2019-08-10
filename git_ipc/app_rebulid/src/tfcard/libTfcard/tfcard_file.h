

#ifndef _TFCARD_FILE_H_
#define _TFCARD_FILE_H_

#ifdef __cplusplus
extern "C"{
#endif


NK_Size TFFILE_GetLocaltime();
NK_Int  TFFILE_RemoveFile(NK_PChar pFilePath);
NK_Int  TFFILE_RemoveDir(NK_PChar pDirPath);
NK_Int  TFFILE_MakeDir(NK_PChar sPathName);
NK_Int TFFILE_HasFile(NK_PChar path);

NK_Int  TFFILE_GetOldDir(NK_PChar pFilePath, NK_PChar pOldDirPath,NK_PChar pOldDirName);
NK_Int  TFFILE_RemoveOldfile(NK_PChar pFilePath, NK_Int nTotal);
NK_Int TFFILE_PareFileName(NK_PChar pstrFileName, NK_Size* nSecStart ,NK_Size* nUsedTime, NK_Int* nFileMode);

#ifdef __cplusplus
}
#endif

#endif

