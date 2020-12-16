package com.nexstreaming.app.common.norm;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Subclass this to define tables in a NormDb.
 *
 * See the comment at the top of NormDb for more usage details.
 */
public abstract class NormTable {

    /**
     * Causes a field to be defined as "UNIQUE" in the underlying SQLite database.
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface Unique {
    }

    /**
     * Causes an index to be defined on this field in the underlying SQLite database.
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface Index {
    }

    /**
     * Causes a field to be defined as "NOT NULL" in the underlying SQLite database.
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface NotNull {
    }

    /**
     * Indicates a field that references an object stored in another table in the database.
     *
     * The field must be defined with a type that is a subclass of NormTable.
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface Foreign {
    }

    @Retention(RetentionPolicy.RUNTIME)
    public @interface PrimaryKey {
    }

    /**
     * For Bitmap fields, causes the bitmap to be compressed as JPEG.
     *
     * Currently, the quality 85 is hard-coded; this may be configurable
     * in a future version.
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface CompressAsJpeg {
    }

    /**
     * For Bitmap fields, causes the bitmap to be compressed as PNG (this
     * is the default)
     */
    @Retention(RetentionPolicy.RUNTIME)
    public @interface CompressAsPng {
    }

    @Retention(RetentionPolicy.RUNTIME)
    public @interface AddColumn {
        int value() default 11;
    }

    private final NormTableInfo tableInfo;
    boolean addedOrUpdatedToDb = false;

    public NormTable() {
        tableInfo = NormTableInfo.getTableInfo(this.getClass());
    }

    public NormTableInfo getTableInfo() {
        return tableInfo;
    }

    public boolean wasAddedToDb() {
        return addedOrUpdatedToDb;
    }

    public long getDbRowID() {
        try {
            return getTableInfo().getIndexColumn().field.getLong(this);
        } catch (IllegalAccessException e) {
            throw new IllegalStateException(e);
        }
    }

    @Override
    public boolean equals(Object o) {
        if( !(o instanceof NormTable) ) {
            return false;
        }
        if( this==o ) {
            return true;
        }
        NormTable other = (NormTable)o;
        if( !other.getTableInfo().getTableName().equals(getTableInfo().getTableName()) ) {
            return false;
        }
        return getDbRowID() == other.getDbRowID();
    }

    @Override
    public int hashCode() {
        return (int) (getDbRowID() * getTableInfo().getTableName().hashCode());
    }
}
