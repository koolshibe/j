// // #include "main.h"

#include "lemlib/api.hpp"
#include "lemlib-tarball/api.hpp"
#include "pros/apix.h"
#include "utest.h"

UTEST_STATE();

extern "C" {

void vexSystemExitRequest(void);
}

void test_initialize() {
    pros::c::serctl(SERCTL_DISABLE_COBS, nullptr);
    pros::delay(100); // wait for the console to print
    utest_main(0, nullptr);
    pros::delay(100); // wait for the console to print
    vexSystemExitRequest();
}

class DecoderStub : public lemlib_tarball::Decoder {
    public:
        DecoderStub(const asset& tarball);

        std::vector<std::string> getPaths();

        std::vector<asset> getAssets();

        std::string getPathName(size_t index);

        std::string getAssetContent(size_t index);
};

// left motor group
pros::MotorGroup test_left_motor_group({-1}, pros::MotorGears::green);
// right motor group
pros::MotorGroup test_right_motor_group({4}, pros::MotorGears::green);
// intake motor
pros::Motor test_intake_motor(7, pros::MotorGears::green);

// drivetrain settings
lemlib::Drivetrain test_drivetrain(&test_left_motor_group, // left motor group
                                   &test_right_motor_group, // right motor group
                                   10, // 10 inch track width
                                   lemlib::Omniwheel::NEW_4, // using new 4" omnis
                                   360, // drivetrain rpm is 360
                                   2 // horizontal drift is 2 (for now)
);

// odometry settings
lemlib::OdomSensors test_sensors(nullptr, // vertical tracking wheel 1, set to null
                                 nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                                 nullptr, // horizontal tracking wheel 1
                                 nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                                 nullptr // inertial sensor
);

// lateral PID controller
lemlib::ControllerSettings test_lateral_controller(10, // proportional gain (kP)
                                                   0, // integral gain (kI)
                                                   3, // derivative gain (kD)
                                                   3, // anti windup
                                                   1, // small error range, in inches
                                                   100, // small error range timeout, in milliseconds
                                                   3, // large error range, in inches
                                                   500, // large error range timeout, in milliseconds
                                                   20 // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings test_angular_controller(2, // proportional gain (kP)
                                                   0, // integral gain (kI)
                                                   10, // derivative gain (kD)
                                                   3, // anti windup
                                                   1, // small error range, in degrees
                                                   100, // small error range timeout, in milliseconds
                                                   3, // large error range, in degrees
                                                   500, // large error range timeout, in milliseconds
                                                   0 // maximum acceleration (slew)
);

// create the chassis
lemlib::Chassis test_chassis(test_drivetrain, // drivetrain settings
                             test_lateral_controller, // lateral PID settings
                             test_angular_controller, // angular PID settings
                             test_sensors // odometry sensors
);

ASSET(my_lemlib_tarball_file_txt);

UTEST(integration_test, chassis_follow_1) {
    DecoderStub decoder(my_lemlib_tarball_file_txt);

    test_chassis.calibrate();
    test_chassis.setPose(40, 0, 0);

#define ASSERT_TIME_TAKEN(statement, time_larger, time_smaller)                                                        \
    {                                                                                                                  \
        long start = pros::millis();                                                                                   \
        statement;                                                                                                     \
        long end = pros::millis();                                                                                     \
        printf("Time taken: %ldms\n", end - start);                                                                    \
        ASSERT_TRUE(end - start >= time_larger);                                                                       \
        ASSERT_TRUE(end - start <= time_smaller);                                                                      \
    }

    ASSERT_TIME_TAKEN(test_chassis.follow(decoder["Path 1"], 1, 1000, true, false), 1000, 1050);
    ASSERT_TIME_TAKEN(test_chassis.follow(decoder["Path 2"], 1, 800, true, false), 800, 850);
    // [LemLib] ERROR: Path not found: does not exist
    // [LemLib] ERROR: Failed to read path file! Are you using the right format? Raw line:
    // [LemLib] ERROR: No points in path! Do you have the right format? Skipping motion
    ASSERT_TIME_TAKEN(test_chassis.follow(decoder["does not exist"], 1, 900, true, false), 0, 50);
}

UTEST(integration_test, chassis_follow_2) {
    DecoderStub decoder(my_lemlib_tarball_file_txt);

    test_chassis.calibrate();
    test_chassis.setPose(40, 0, 0);

    ASSERT_FALSE(test_chassis.isInMotion());
    test_chassis.follow(decoder["Path 1"], 1, 1000, true, true); // async
    ASSERT_TRUE(test_chassis.isInMotion());
    for (int i = 0; i < 100; i++) {
        test_chassis.setPose(40, 44, 0);
        pros::delay(10); // yield to the async task
    }
    ASSERT_FALSE(test_chassis.isInMotion());

    test_chassis.setPose(40, 23.622, 0);

    ASSERT_FALSE(test_chassis.isInMotion());
    test_chassis.follow(decoder["Path 2"], 1, 1000, true, true); // async
    ASSERT_TRUE(test_chassis.isInMotion());
    for (int i = 0; i < 100; i++) {
        test_chassis.setPose(-43.994, 36.502, 0);
        pros::delay(10); // yield to the async task
    }
    ASSERT_FALSE(test_chassis.isInMotion());
}
