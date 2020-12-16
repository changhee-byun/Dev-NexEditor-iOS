package com.nexstreaming.app.common.norm;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteFullException;
import android.database.sqlite.SQLiteOpenHelper;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import com.google.gson_nex.Gson;

import java.io.ByteArrayOutputStream;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * An object-relational mapping (ORM) for Android.
 *
 * The name "Norm" is from "NexStreaming ORM".
 *
 * This was originally created by Matthew Feinberg
 * (direct inquiries to matthew.feinberg@nexstreaming.com)
 *
 * Before using this, consider other ORM options:
 * - If you need high performance, consider 'greedDAO' instead
 * - If you need something that runs on any Java implementation, not only Android, consider 'OrmLite'
 *
 * Where Norm is good:
 * - Easy to use (for cases where code organization is more important than performance)
 * - Designed for Android (allows fields to be defined holding Bitmap, etc.)
 * - Multi-instance
 * - Doesn't require Manifest changes (unlike Sugar ORM, for example), which makes it
 *    suitable for use in libraries and SDKs.
 * - Fields can contain arrays and maps; these are stored as JSON strings (but if you have really
 *    big ones, make a separate table with a relationship).
 *
 * Things that need improvement:
 * - Foreign fields:  The referenced object must be saved in the database first before the
 *    referencing object is saved.  If you don't do that, the relationship is lost silently.
 *    Need to throw an exception or offer the option to automatically create the referenced object.
 * - Certain illegal cases are not detected and fail silently rather than throwing an exception.
 *    For example, field names that match SQL reserved keywords are not handled properly, and field
 *    names that conflict after conversion to underscore form have undefined (read: bad) behavior.
 *
 * To use:
 *   1) subclass NormTable for each table you want in the database.
 *          - Exactly one primary index field must be present.  Index field must be defined as,
 *              a long and either named "_id", OR have the @PrimaryKey annotation.
 *   2) subclass NormDb; you only need to implement two methods:
 *          - The constructor should accept a single Context argument and
 *              call super() passing the Context, as well as a database
 *
 */
public abstract class NormDb extends SQLiteOpenHelper {

    private static final String LOG_TAG = "NormDb";

    public NormDb(Context context, String databaseName, int schemaVersion) {
        super(context,databaseName,null,schemaVersion);
    }

    protected abstract Class<? extends NormTable>[] getTableClasses();
    private final Gson gson = new Gson();

    @Override
    public void onCreate(SQLiteDatabase db) {
//        StringBuilder createEntries = new StringBuilder();
        for( Class<? extends NormTable> tableClass: getTableClasses() ) {
            String[] createSql = NormTableInfo.getTableInfo(tableClass).getCreateTableSQL();
            for( String s: createSql ) {
                Log.d(LOG_TAG, "CreateTable:" + s );
                db.execSQL(s);
                Log.d(LOG_TAG, "Created");
            }
//            createEntries.append(NormTableInfo.getTableInfo(tableClass).getCreateTableSQL()).append(';').append("\n");
        }
//        Log.d(LOG_TAG, "CreateTables:" + createEntries.toString() );
//        db.execSQL(createEntries.toString());
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
//        StringBuilder dropEntries = new StringBuilder();
        Log.d(LOG_TAG, "onUpgrade oldVersion="+oldVersion+", newVersion="+newVersion);
        if( oldVersion < 11 ){
            for (Class<? extends NormTable> tableClass : getTableClasses()) {

                String[] dropTableSql = NormTableInfo.getTableInfo(tableClass).getDropTableSQL();
                for (String s : dropTableSql) {
                    Log.d(LOG_TAG, "DropTable:" + s);
                    db.execSQL(s);
                    Log.d(LOG_TAG, "Dropped");
                }
            }
            onCreate(db);
            return;
        }

        if( oldVersion <  newVersion ) {

            for (Class<? extends NormTable> tableClass : getTableClasses()) {
                String[] addColumnSql = NormTableInfo.getTableInfo(tableClass).getUpgradeTableAddColumnSQL(oldVersion, newVersion);
                for (String s : addColumnSql) {
                    Log.d(LOG_TAG, "onUpgrade:" + s);
                    db.execSQL(s);
                    Log.d(LOG_TAG, "onUpgrade end");
                }
            }
        }
    }

    @Override
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        //super.onDowngrade(db, oldVersion, newVersion);
        for (Class<? extends NormTable> tableClass : getTableClasses()) {

            String[] dropTableSql = NormTableInfo.getTableInfo(tableClass).getDropTableSQL();
            for (String s : dropTableSql) {
                Log.d(LOG_TAG, "onDowngrade:" + s);
                db.execSQL(s);
                Log.d(LOG_TAG, "onDowngrade");
            }
        }
        onCreate(db);
    }

    public static String camelCaseToLCUnderscore(String s) {
        String lc = s.replaceAll("(?<=[A-Z])(?=[A-Z][a-z])|(?<=[^A-Z])(?=[A-Z])|(?<=[A-Za-z])(?=[^A-Za-z])","_").toLowerCase(Locale.ENGLISH);
        if( lc.length() < 1 )
            return "_";
        char c = lc.charAt(0);
        if( c != '_' && (c < 'a' || c > 'z') ) {
            return "_" + lc;
        } else {
            return lc;
        }
    }

    private  <T extends NormTable> ContentValues getContentValuesForRecord(T record) {
        ContentValues initialValues = new ContentValues();
        NormColumnInfo[] columnInfo = record.getTableInfo().getColumns();
        try {
            for (NormColumnInfo col : columnInfo) {
                if( col==null )
                    continue;
                if( col.rowId )
                    continue;
                switch (col.columnType) {
                    case TEXT:
                        Object obj = col.field.get(record);
                        if( obj==null ) {
                            initialValues.putNull(col.columnName);
                        } else {
                            initialValues.put(col.columnName, String.valueOf(obj));
                        }
                        break;
                    case INT:
                        initialValues.put(col.columnName, col.field.getInt(record));
                        break;
                    case LONG:
                        if( col.foreign ) {
                            Object value = col.field.get(record);
                            if( value instanceof NormTable ) {
                                initialValues.put(col.columnName, ((NormTable) value).getDbRowID());
                            }
                        } else {
                            initialValues.put(col.columnName, col.field.getLong(record));
                        }
                        break;
                    case DOUBLE:
                        initialValues.put(col.columnName,col.field.getDouble(record));
                        break;
                    case FLOAT:
                        initialValues.put(col.columnName,col.field.getFloat(record));
                        break;
                    case ENUM: {
                        Enum value = ((Enum)col.field.get(record));
                        initialValues.put(col.columnName, value == null ? null : value.name());
                        break;
                    }
                    case BOOL:
                        initialValues.put(col.columnName,col.field.getBoolean(record)?1:0);
                        break;
                    case BLOB: {
                        byte[] value = (byte[]) col.field.get(record);
                        initialValues.put(col.columnName, value);
                        break;
                    }
                    case PNG: {
                        byte[] value = null;
                        Bitmap bm = (Bitmap) col.field.get(record);
                        if( bm!=null ) {
                            ByteArrayOutputStream out = new ByteArrayOutputStream();
                            bm.compress(Bitmap.CompressFormat.PNG, 100, out );
                            value = out.toByteArray();
                        }
                        initialValues.put(col.columnName, value);
                        break;
                    }
                    case JPEG: {
                        byte[] value = null;
                        Bitmap bm = (Bitmap) col.field.get(record);
                        if( bm!=null ) {
                            ByteArrayOutputStream out = new ByteArrayOutputStream();
                            bm.compress(Bitmap.CompressFormat.JPEG, 85, out );
                            value = out.toByteArray();
                        }
                        initialValues.put(col.columnName, value);
                        break;
                    }
                    case JSON:
                        initialValues.put(col.columnName,gson.toJson(col.field.get(record)));
                        break;
                }
            }
        } catch (IllegalAccessException e) {
            throw new IllegalStateException(e);
        }
        return initialValues;
    }

    private <T extends NormTable> void updateRecIndex(T record, long index) {
        try {
            record.getTableInfo().getIndexColumn().field.setLong(record,index);
            record.addedOrUpdatedToDb = true;
        } catch (IllegalAccessException e) {
            throw new IllegalStateException(e);
        }
    }

    public <T extends NormTable> void addOrUpdate(T record) {
        ContentValues initialValues = getContentValuesForRecord(record);
        long rowId = getWritableDatabase().insertWithOnConflict(record.getTableInfo().getTableName(), null, initialValues, SQLiteDatabase.CONFLICT_REPLACE);
        updateRecIndex(record,rowId);
    }

    public <T extends NormTable> void add(T record) {
        ContentValues initialValues = getContentValuesForRecord(record);
        long rowId = getWritableDatabase().insert(record.getTableInfo().getTableName(), null, initialValues);
        updateRecIndex(record,rowId);
    }

    public <T extends NormTable> void update(T record) {
        ContentValues initialValues = getContentValuesForRecord(record);
        try {
            long rowId = getWritableDatabase().update(record.getTableInfo().getTableName(), initialValues,
                    "_id = ?", new String[]{String.valueOf(record.getTableInfo().getIndexColumn().field.getLong(record))});
        } catch (IllegalAccessException e) {
            throw new IllegalStateException(e);
        }
    }

    public <T extends NormTable> T findById(Class<T> tableClass, long id) {
        List<T> result =  query(tableClass, NormTableInfo.getTableInfo(tableClass).getIndexColumn().columnName + " = ? LIMIT 1", id);
        if( result!=null && !result.isEmpty() ) {
            return result.get(0);
        } else {
            return null;
        }
    }

    public <T extends NormTable> T findFirst(Class<T> tableClass, String query, Object... params) {
        List<T> result =  query(tableClass, query + " LIMIT 1", params);
        if( result!=null && !result.isEmpty() ) {
            return result.get(0);
        } else {
            return null;
        }
    }

    public <T extends NormTable> long findFirstRowId(Class<T> tableClass, String query, Object... params) {
        List<T> result =  query(tableClass,query + " LIMIT 1", params);
        if( result!=null && !result.isEmpty() ) {
            return result.get(0).getDbRowID();
        } else {
            return 0;
        }
    }

    public <T extends NormTable> List<T> query(Class<T> tableClass) {
        return query_internal(tableClass, null,null,true);
    }

    public <T extends NormTable> void delete(Class<T> tableClass, String query, Object... params) {
        String[] queryParams = new String[params.length];
        for( int i=0; i<params.length; i++ ) {
            queryParams[i] = String.valueOf(params[i]);
        }
        getWritableDatabase().delete(NormTableInfo.getTableInfo(tableClass).getTableName(),query,queryParams);
    }

    public <T extends NormTable> void delete(T record) {
        getWritableDatabase().delete(record.getTableInfo().getTableName(),record.getTableInfo().getIndexColumn() + " = ?", new String[] {String.valueOf(record.getDbRowID())});
    }

    public <T extends NormTable> List<T> query(Class<T> tableClass, String query, Object... params) {
        return query_internal(tableClass, query, params,true);
    }

    public <T extends NormTable> int count(Class<T> tableClass) {
        return count_internal(tableClass, null,null,false);
    }

    public <T extends NormTable> int count(Class<T> tableClass, String query, Object... params) {
        return count_internal(tableClass, query, params,false);
    }

    private <T extends NormTable> int count_internal(Class<T> tableClass, String query, Object[] params, boolean getForeignKeys) {
        String[] queryParams = null;
        if(params!=null) {
            queryParams = new String[params.length];
            for (int i = 0; i < params.length; i++) {
                queryParams[i] = String.valueOf(params[i]);
            }
        }
        NormTableInfo tableInfo = NormTableInfo.getTableInfo(tableClass);
        String[] columnNames = {tableInfo.getIndexColumn().columnName};
        Cursor cursor = getReadableDatabase().query(tableInfo.getTableName(), columnNames, query, queryParams, null, null, null);
        int result = cursor.getCount();
        cursor.close();
        return result;
    }

    private <T extends NormTable> List<T> query_internal(Class<T> tableClass, String query, Object[] params, boolean getForeignKeys) {
        String[] queryParams = null;
        if(params!=null) {
            queryParams = new String[params.length];
            for (int i = 0; i < params.length; i++) {
                queryParams[i] = String.valueOf(params[i]);
            }
        }
        NormTableInfo tableInfo = NormTableInfo.getTableInfo(tableClass);
        NormColumnInfo columnInfo[];
        String[] columnNames;
        columnInfo = tableInfo.getColumns();
        columnNames = tableInfo.getColumnNames();
        Cursor cursor = getReadableDatabase().query(tableInfo.getTableName(), columnNames, query, queryParams, null, null, null);
        List<T> result = new ArrayList<>();
        Map<Class<? extends NormTable>,Map<Long,Object>> foreignObjectCache = new HashMap<>();
        try {
            cursor.moveToPosition(-1);
            while( cursor.moveToNext() ) {
                T entry = tableClass.newInstance();
                for( int i=0; i<columnInfo.length; i++ ) {
                    switch( columnInfo[i].columnType ) {
                        case INT:
                            columnInfo[i].field.setInt(entry, cursor.getInt(i));
                            break;
                        case LONG:
                            if( columnInfo[i].foreign ) {
                                if(getForeignKeys) {
                                    if( !foreignObjectCache.containsKey(columnInfo[i].foreignClass)) {
                                        foreignObjectCache.put(columnInfo[i].foreignClass,new HashMap<Long, Object>());
                                    }
                                    long foreignIndex = cursor.getLong(cursor.getColumnIndex(columnInfo[i].columnName));
                                    Object resultObject = foreignObjectCache.get(columnInfo[i].foreignClass).get(foreignIndex);
                                    if( resultObject==null ) {
                                        resultObject = findById(columnInfo[i].foreignClass, foreignIndex);
                                        foreignObjectCache.get(columnInfo[i].foreignClass).put(foreignIndex,resultObject);
                                    }
                                    columnInfo[i].field.set(entry, resultObject);
                                }
                            } else {
                                columnInfo[i].field.setLong(entry, cursor.getLong(i));
                            }
                            break;
                        case DOUBLE:
                            columnInfo[i].field.setDouble(entry, cursor.getDouble(i));
                            break;
                        case FLOAT:
                            columnInfo[i].field.setFloat(entry, cursor.getFloat(i));
                            break;
                        case BOOL:
                            columnInfo[i].field.setBoolean(entry, cursor.getInt(i) != 0);
                            break;
                        case TEXT:
                            columnInfo[i].field.set(entry, cursor.getString(i));
                            break;
                        case BLOB:
                            columnInfo[i].field.set(entry, cursor.getBlob(i));
                            break;
                        case JPEG:
                        case PNG: {
                            byte[] value = cursor.getBlob(i);
                            Bitmap bm = BitmapFactory.decodeByteArray(value,0,value.length);
                            columnInfo[i].field.set(entry, bm);
                            break;
                        }
                        case JSON: {
                            String rawJSON = cursor.getString(i);
                            columnInfo[i].field.set(entry,gson.fromJson(rawJSON,columnInfo[i].field.getGenericType()));
                            break;
                        }
                        case ENUM: {
                            Type fieldType = columnInfo[i].field.getType();
                            Class<? extends Enum> enumType = (Class<? extends Enum>) fieldType;
                            String enumName = cursor.getString(i);
                            if( enumName!=null ) {
                                try {
                                    columnInfo[i].field.set(entry, Enum.valueOf(enumType, enumName));
                                } catch (IllegalArgumentException e) {
                                    columnInfo[i].field.set(entry, null);
                                }
                            }
                        }
                    }
                }
                result.add(entry);
            }
        } catch (InstantiationException e) {
            throw new IllegalStateException(e);
        } catch (IllegalAccessException e) {
            throw new IllegalStateException(e);
        } finally {
            cursor.close();
        }
        return result;
    }

    public <T extends NormTable, RESULT_TYPE> List<RESULT_TYPE> queryIndividualField(Class<T> tableClass, String fieldName, String query, Object... params) {
        String[] queryParams = null;
        if(params!=null) {
            queryParams = new String[params.length];
            for (int i = 0; i < params.length; i++) {
                queryParams[i] = String.valueOf(params[i]);
            }
        }
        NormTableInfo tableInfo = NormTableInfo.getTableInfo(tableClass);
        NormColumnInfo columnInfo;
        String[] columnNames;
        columnInfo = tableInfo.getColumnFromName(fieldName);
        columnNames = new String[]{fieldName};
        Cursor cursor = getReadableDatabase().query(tableInfo.getTableName(), columnNames, query, queryParams, null, null, null);
        List<RESULT_TYPE> result = new ArrayList<>();
        Map<Class<? extends NormTable>,Map<Long,Object>> foreignObjectCache = new HashMap<>();
        try {
            cursor.moveToPosition(-1);
            while( cursor.moveToNext() ) {
                RESULT_TYPE resultEntry = null;
                switch( columnInfo.columnType ) {
                    case INT:
                        resultEntry = (RESULT_TYPE)(Integer)cursor.getInt(0);
                        break;
                    case LONG:
                        if( columnInfo.foreign ) {
                            if( !foreignObjectCache.containsKey(columnInfo.foreignClass)) {
                                foreignObjectCache.put(columnInfo.foreignClass,new HashMap<Long, Object>());
                            }
                            long foreignIndex = cursor.getLong(0);
                            Object resultObject = foreignObjectCache.get(columnInfo.foreignClass).get(foreignIndex);
                            if( resultObject==null ) {
                                resultObject = findById(columnInfo.foreignClass, foreignIndex);
                                foreignObjectCache.get(columnInfo.foreignClass).put(foreignIndex,resultObject);
                            }
                            resultEntry = (RESULT_TYPE)resultObject;
                        } else {
                            resultEntry = (RESULT_TYPE)(Long)cursor.getLong(0);
                        }
                        break;
                    case DOUBLE:
                        resultEntry = (RESULT_TYPE)(Double)cursor.getDouble(0);
                        break;
                    case FLOAT:
                        resultEntry = (RESULT_TYPE)(Float)cursor.getFloat(0);
                        break;
                    case BOOL:
                        resultEntry = (RESULT_TYPE)(Boolean)(cursor.getInt(0) != 0);
                        break;
                    case TEXT:
                        resultEntry = (RESULT_TYPE)cursor.getString(0);
                        break;
                    case BLOB:
                        resultEntry = (RESULT_TYPE)cursor.getBlob(0);
                        break;
                    case JPEG:
                    case PNG: {
                        byte[] value = cursor.getBlob(0);
                        Bitmap bm = BitmapFactory.decodeByteArray(value,0,value.length);
                        resultEntry = (RESULT_TYPE)bm;
                        break;
                    }
                    case JSON: {
                        String rawJSON = cursor.getString(0);
                        resultEntry = (RESULT_TYPE) gson.fromJson(rawJSON, columnInfo.field.getGenericType());
                        break;
                    }
                    case ENUM: {
                        Type fieldType = columnInfo.field.getType();
                        Class<? extends Enum> enumType = (Class<? extends Enum>) fieldType;
                        String enumName = cursor.getString(0);
                        if( enumName!=null ) {
                            try {
                                resultEntry = (RESULT_TYPE)Enum.valueOf(enumType, enumName);
                            } catch (IllegalArgumentException e) {
                                resultEntry = null;
                            }
                        }
                    }
                }
                result.add(resultEntry);
            }
        } finally {
            cursor.close();
        }
        return result;
    }

    public void beginTransaction() {
        getWritableDatabase().beginTransaction();
    }

    public void setTransactionSuccessful() {
        getWritableDatabase().setTransactionSuccessful();
    }

    public void endTransaction() throws SQLiteFullException{
		try {
            getWritableDatabase().endTransaction();
        } catch (SQLiteFullException e) {
            throw new SQLiteFullException();
        }
    }

}
