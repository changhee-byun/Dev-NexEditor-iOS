package com.nexstreaming.app.common.nexasset.assetpackage.db;


import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;
import com.nexstreaming.app.common.norm.NormTable;

public class InstallSourceRecord extends NormTable {
    public long _id;
    @NormTable.Unique
    @NormTable.NotNull
    public String installSourceId;
    public InstallSourceType installSourceType;
    public long installSourceVersion;

}
