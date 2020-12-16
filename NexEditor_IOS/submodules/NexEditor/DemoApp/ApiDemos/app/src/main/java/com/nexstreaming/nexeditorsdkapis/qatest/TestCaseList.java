package com.nexstreaming.nexeditorsdkapis.qatest;

import com.nexstreaming.nexeditorsdkapis.qatest.tc.AudioTest;
import com.nexstreaming.nexeditorsdkapis.qatest.tc.ClipInfoTest;
import com.nexstreaming.nexeditorsdkapis.qatest.tc.ExportTest;
import com.nexstreaming.nexeditorsdkapis.qatest.tc.SeekTest;

/**
 * Created by jeongwook.yoon on 2016-05-10.
 */
public class TestCaseList {
    static final Class sTestCaseList[] = {
            SeekTest.class,
            ExportTest.class,
            ClipInfoTest.class,
            AudioTest.class,
    };
}
