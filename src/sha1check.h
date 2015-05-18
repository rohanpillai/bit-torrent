#ifndef _SHACHECK_H_
#define _SHACHECK_H_
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <iostream>
#include <parser.h>
#include <types.h>

unsigned char *getSHA1(char *);
unsigned char *getSHA1(std::string);
#endif

