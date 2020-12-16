package com.nexstreaming.app.common.nexasset.assetpackage.db;

import android.content.Context;

import com.nexstreaming.app.common.norm.NormDb;
import com.nexstreaming.app.common.norm.NormTable;

public class AssetPackageDb extends NormDb {

    public AssetPackageDb(Context context) {
        super(context, "com.kinemaster.asset_package.db", 13);
    }

    @Override
    protected Class<? extends NormTable>[] getTableClasses() {
        return new Class[] {
            AssetPackageRecord.class,
            ItemRecord.class,
            CategoryRecord.class,
            SubCategoryRecord.class,
            InstallSourceRecord.class
        };
    }
}
