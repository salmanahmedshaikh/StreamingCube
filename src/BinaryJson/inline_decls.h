//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"

#ifdef WINDOWS
#define NOINLINE_DECL __declspec(noinline)
#else
#define NOINLINE_DECL __attribute__((noinline))
#endif
#define PACKED_DECL

// branch prediction.  indicate we expect to be false
# define MONGO_unlikely(x) ((bool)(x))
