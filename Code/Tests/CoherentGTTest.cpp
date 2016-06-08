#include "StdAfx.h"

#include <gtest/gtest.h>

class CoherentGTTest : public ::testing::Test
{
protected:
    void SetUp() override
    {

    }

    void TearDown() override
    {

    }
};

TEST_F(CoherentGTTest, ExampleTest)
{
    ASSERT_TRUE(true);
}

GEM_IMPLEMENT_TEST_RUNNER
