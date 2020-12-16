package com.nexstreaming.app.common.norm;

import android.graphics.Bitmap;

import java.lang.reflect.Field;
import java.util.Collection;
import java.util.Map;

public class NormColumnInfo {
    final String columnName;
    final Field field;
    final ColumnType columnType;
    final boolean unique;
    final boolean notNull;
    final boolean foreign;
    final boolean rowId;
    final boolean index;
    final int addColumnVersion;
    final Class<? extends NormTable> foreignClass;

    NormColumnInfo(Field field) {
        ColumnType columnType;
        Class<?> columnClass = field.getType();
        this.foreign = field.isAnnotationPresent(NormTable.Foreign.class) || NormTable.class.isAssignableFrom(columnClass);
        if( this.foreign && NormTable.class.isAssignableFrom(columnClass)) {
            foreignClass = (Class<? extends NormTable>) columnClass;
        } else {
            foreignClass = null;
        }
        rowId = field.getName().equals("_id") || field.isAnnotationPresent(NormTable.PrimaryKey.class);
        if (this.foreign)
        {
            columnType = ColumnType.LONG;
        } else if (columnClass == Integer.class || columnClass == Integer.TYPE) {
            columnType = ColumnType.INT;
        } else if (columnClass == Long.class || columnClass == Long.TYPE) {
            columnType = ColumnType.LONG;
        } else if (columnClass == Float.class || columnClass == Float.TYPE) {
            columnType = ColumnType.FLOAT;
        } else if (columnClass == Double.class || columnClass == Double.TYPE) {
            columnType = ColumnType.DOUBLE;
        } else if (columnClass == String.class) {
            columnType = ColumnType.TEXT;
        } else if (columnClass == Boolean.class || columnClass == Boolean.TYPE) {
            columnType = ColumnType.BOOL;
        } else if (Collection.class.isAssignableFrom(columnClass)) {
            columnType = ColumnType.JSON;
        } else if (Map.class.isAssignableFrom(columnClass)) {
            columnType = ColumnType.JSON;
        } else if (columnClass == byte[].class) {
            columnType = ColumnType.BLOB;
        } else if (columnClass == Bitmap.class) {
            if( field.isAnnotationPresent(NormTable.CompressAsJpeg.class) ) {
                columnType = ColumnType.JPEG;
            } else {
                columnType = ColumnType.PNG;
            }
        } else if (Enum.class.isAssignableFrom(columnClass)) {
            columnType = ColumnType.ENUM;
        } else {
            throw new UnsupportedOperationException("Unrecognized column type: " + columnClass.getSimpleName() + " (for column '" + field.getName() + "')");
        }
        if( rowId && columnType!= ColumnType.LONG ) {
            throw new IllegalArgumentException("Primary key must be 'long' type.");
        }
        this.unique = field.isAnnotationPresent(NormTable.Unique.class);
        this.index = field.isAnnotationPresent(NormTable.Index.class);
        this.notNull = field.isAnnotationPresent(NormTable.NotNull.class);
        this.columnName = NormDb.camelCaseToLCUnderscore(field.getName());
        this.field = field;
        this.columnType = columnType;

        if( field.isAnnotationPresent(NormTable.AddColumn.class) ){
            this.addColumnVersion = field.getAnnotation(NormTable.AddColumn.class).value();
        }else{
            this.addColumnVersion = 11;
        }
    }

    public static enum ColumnType {
        INT, LONG, FLOAT, DOUBLE, TEXT, BOOL, JSON, ENUM, BLOB, JPEG, PNG
    }

    @Override
    public String toString() {
        return columnName;
    }
}
