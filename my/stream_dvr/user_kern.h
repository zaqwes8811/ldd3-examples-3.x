/*
 * user_kern.h
 *
 *  Created on: Mar 21, 2016
 *      Author: zaqwes
 */

#ifndef USER_KERN_H_
#define USER_KERN_H_

// https://habrahabr.ru/post/142662/
// http://www.catb.org/esr/structure-packing/
//   "align. decrease"
#pragma pack(push, 1)
struct img_hndl_t
{
	int id;

	int x;
	int y;
};
#pragma pack(pop)


#endif /* USER_KERN_H_ */
