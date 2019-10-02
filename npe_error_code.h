//  Copyright (c) 2017-2019 by North Pole Engineering, Inc.  All rights reserved.
//
//  Printed in the United States of America.  Except as permitted under the United States
//  Copyright Act of 1976, no part of this software may be reproduced or distributed in
//  any form or by any means, without the prior written permission of North Pole
//  Engineering, Inc., unless such copying is expressly permitted by federal copyright law.
//
//  Address copying inquires to:
//  North Pole Engineering, Inc.
//  npe@npe-inc.com
//  221 North First St. Ste. 310
//  Minneapolis, Minnesota 55401
//
//  Information contained in this software has been created or obtained by North Pole Engineering,
//  Inc. from sources believed to be reliable.  However, North Pole Engineering, Inc. does not
//  guarantee the accuracy or completeness of the information published herein nor shall
//  North Pole Engineering, Inc. be liable for any errors, omissions, or damages arising
//  from the use of this software.
#ifndef __NPE_ERR_CODES__
#define __NPE_ERR_CODES__
#include <assert.h>

#define NPE_GEM_RESPONSE_OK                  ((uint32_t) 0)
#define NPE_GEM_RESPONSE_TIMEOUT_OUT         ((uint32_t) 1)
#define NPE_GEM_RESPONSE_RETRIES_EXHAUSTED   ((uint32_t) 2)
#define NPE_GEM_RESPONSE_UNABLE_TO_WRITE     ((uint32_t) 3)
#define NPE_GEM_RESPONSE_SERIAL_NO_COMPORT   ((uint32_t) 4)
#define NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL    ((uint32_t) 5)
#define NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL  ((uint32_t) 6)
#define NPE_GEM_RESPONSE_INVALID_PARAMETER   ((uint32_t) 7)


#endif