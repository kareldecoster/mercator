#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <mercator_processing_unit.h>
#include <mercator_data_acquisition_unit.h>
#include <time.h>
#include <check.h>

#define FRAME_SIZE 1024
#define PHI (300.0/76800.0)	/*Lowest emited frequency deviced by the sampling frequency*/
#define VREF 3.3

double* read_frame(FILE* fp){
	if(fp == NULL){
		return NULL;
	}
	double* data = malloc(FRAME_SIZE*sizeof(double));
	if(data == NULL){
		return NULL;
	}
	if(fread(data, sizeof(double), FRAME_SIZE, fp) < FRAME_SIZE){
		free(data);
		return NULL;
	}
	return data;
	
}

START_TEST(test_mpu_create)
{
    mpu_data_t* mpu;

    mpu = mpu_create(0, PHI);
    ck_assert(mpu == NULL); //Should fail if frame_size is too small.
    mpu = mpu_create(FRAME_SIZE, 0.00);
    ck_assert(mpu == NULL); //Phi is not allowed to be 0.
    mpu = mpu_create(FRAME_SIZE, -1.00);
    ck_assert(mpu == NULL); //Phi is not allowed to be negative.
    mpu = mpu_create(FRAME_SIZE, PHI);
    ck_assert(mpu != NULL); //This should succeed.
    mpu_destroy(&mpu);
    ck_assert(mpu == NULL);
}
END_TEST

START_TEST(test_mpu_calculate_x)
{
	double x_led[4] = {X_0, X_1, X_2, X_3};
	double y_led[4] = {Y_0, Y_1, Y_2, Y_3};
	double d_vec[3] = {1.5007, 1.5487, 2.1560};
	double x_vec[3] = {x_led[0], x_led[1], x_led[3]};
	double y_vec[3] = {y_led[0], y_led[1], y_led[3]};
	double x;
	x = calculate_x(d_vec, x_vec, y_vec);
	ck_assert(((x-0.7013)< 0.0001)&&((0.7013-x)<0.0001));
	x = calculate_x(NULL, x_vec, y_vec);
	ck_assert(isunordered(x, 0.0));
	x = calculate_x(d_vec, NULL, y_vec);
	ck_assert(isunordered(x, 0.0));
	x = calculate_x(d_vec, x_vec, NULL);
	ck_assert(isunordered(x, 0.0));
	d_vec[0]=0.0;
	d_vec[1]=0.0;
	d_vec[2]=0.0;
	x = calculate_x(d_vec, x_vec, y_vec);
	ck_assert(isunordered(x, 0.0));
}
END_TEST

START_TEST(test_mpu_calculate_y)
{
	double x_led[4] = {X_0, X_1, X_2, X_3};
	double y_led[4] = {Y_0, Y_1, Y_2, Y_3};
	double d_vec[3] = {1.5007, 1.5487, 2.1560};
	double x_vec[3] = {x_led[0], x_led[1], x_led[3]};
	double y_vec[3] = {y_led[0], y_led[1], y_led[3]};
	double y;
	y = calculate_y(d_vec, x_vec, y_vec);
	ck_assert(((y-1.451248373622487)< 0.0001)&&((1.451248373622487-y)<0.0001));
	y = calculate_y(NULL, x_vec, y_vec);
	ck_assert(isunordered(y, 0.0));
	y = calculate_y(d_vec, NULL, y_vec);
	ck_assert(isunordered(y, 0.0));
	y = calculate_y(d_vec, x_vec, NULL);
	ck_assert(isunordered(y, 0.0));
	d_vec[0]=0.0;
	d_vec[1]=0.0;
	d_vec[2]=0.0;
	y = calculate_y(d_vec, x_vec, y_vec);
	ck_assert(isunordered(y, 0.0));
}
END_TEST

START_TEST(test_mpu_triangulate)
{
	double x_led[4] = {X_0, X_1, X_2, X_3};
	double y_led[4] = {Y_0, Y_1, Y_2, Y_3};
	double d_vec[3] = {1.5007, 1.5487, 2.1560};
	double x_vec[3] = {x_led[0], x_led[1], x_led[3]};
	double y_vec[3] = {y_led[0], y_led[1], y_led[3]};
	double x,y;
	int i;
	i = triangulate(d_vec, x_vec, y_vec, &x, &y);
	ck_assert(((x-0.7013)<0.01)&&((x-0.7013)<0.01)&&((y-1.451248373622487)<0.0001)&&((1.451248373622487-y)<0.01));
	ck_assert(i==EXIT_SUCCESS);
	i = triangulate(NULL, x_vec, y_vec, &x, &y);	
	ck_assert(i==EXIT_FAILURE);
	i = triangulate(d_vec, NULL, y_vec, &x, &y);	
	ck_assert(i==EXIT_FAILURE);
	i = triangulate(d_vec, x_vec, NULL, &x, &y);	
	ck_assert(i==EXIT_FAILURE);
	i = triangulate(d_vec, x_vec, y_vec, NULL, &y);	
	ck_assert(i==EXIT_FAILURE);
	i = triangulate(d_vec, x_vec, y_vec, &x, NULL);	
	ck_assert(i==EXIT_FAILURE);
	d_vec[0]=0.0;
	d_vec[1]=0.0;
	d_vec[2]=0.0;
	i = triangulate(d_vec, x_vec, y_vec, &x, &y);	
	ck_assert(i==EXIT_FAILURE);
}
END_TEST

START_TEST(test_mpu_execute){
	FILE* fp = fopen("./res/posj.bin","rb");
	double* raw_data =read_frame(fp);
	mpu_data_t* mpu = mpu_create(FRAME_SIZE, PHI);
	ck_assert(mpu!=NULL);
	double* position = mpu_execute(raw_data, mpu);
	double x = position[0];
	double y = position[1];
	ck_assert(((x-0.672279468569579)<0.0001)&&((x-0.672279468569579)<0.0001)&&((y-1.454406408543497)<0.0001)&&((1.454406408543497-y)<0.0001));
	position = mpu_execute(NULL, mpu);
	ck_assert(position==NULL);
	position = mpu_execute(raw_data, NULL);
	ck_assert(position==NULL);
	mpu_destroy(&mpu);
	ck_assert(mpu==NULL);
	free(raw_data);
	fclose(fp);
}
END_TEST

Suite * mpu_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("mpu");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_mpu_create);
    tcase_add_test(tc_core, test_mpu_calculate_x);
    tcase_add_test(tc_core, test_mpu_calculate_y);
    tcase_add_test(tc_core, test_mpu_triangulate);
    tcase_add_test(tc_core, test_mpu_execute);
    suite_add_tcase(s, tc_core);

    return s;
}

START_TEST(test_mdau_create){
	int i;
	i = mdau_create(0, FREQ_76_8kHz, VREF);
	ck_assert(i!=EXIT_SUCCESS);
	i = mdau_create(FRAME_SIZE, FREQ_76_8kHz, 0.0);
	ck_assert(i!=EXIT_SUCCESS);
	i = mdau_create(FRAME_SIZE, FREQ_76_8kHz,VREF);
	ck_assert(i==EXIT_SUCCESS);
	i=mdau_destroy();
	ck_assert(i==EXIT_SUCCESS);
}
END_TEST

Suite * mdau_suite(void){
	Suite* s;
	TCase* tc_core;
	
	s = suite_create("mdau");
	
	/* Core test case */
	tc_core = tcase_create("Core");
	
	tcase_add_test(tc_core, test_mdau_create);
	suite_add_tcase(s, tc_core);
	
	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = mpu_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	s = mdau_suite();
	sr = srunner_create(s);
	
	srunner_run_all(sr, CK_NORMAL);
	number_failed += srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
