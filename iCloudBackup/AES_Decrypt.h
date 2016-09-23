#pragma once

#include <iostream>  
#include "AES.h"  
#include "Base64.h"  
using namespace std;

const char g_key[17] = "huduniCloudHudun";
const char g_iv[17] = "\0";

string EncryptionAES(const string& strSrc); //AESº”√‹  
string DecryptionAES(const string& strSrc); //AESΩ‚√‹  