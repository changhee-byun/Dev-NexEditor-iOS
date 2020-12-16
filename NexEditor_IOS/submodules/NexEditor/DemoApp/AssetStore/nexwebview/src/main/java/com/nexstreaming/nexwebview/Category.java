package com.nexstreaming.nexwebview;

/**
 * Created by jeongwook.yoon on 2016-11-16.
 */

public class Category {
    int idx;
    String aliasName;
    String categoryUrl;
    Category(int idx, String aliasName, String categoryUrl){
        this.idx = idx;
        this.aliasName = aliasName;
        this.categoryUrl = categoryUrl;
    }

    public String getAliasName() {
        return aliasName;
    }

    public int getIdx() {
        return idx;
    }

    public String getCategoryUrl() {
        return categoryUrl;
    }

    @Override
    public String toString() {
        return "Category{" +
                "aliasName='" + aliasName + '\'' +
                "categoryUrl='" + categoryUrl + '\'' +
                ", idx=" + idx +
                '}';
    }
}
