package com.nexstreaming.app.common.norm;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by matthewfeinberg on 7/27/16.
 */
public class NormTableInfo {


    private final String[] createTableSQL;
    private final String[] dropTableSQL;
    private final NormColumnInfo[] columns;
    private final String tableName;
    private final String[] columnNames;
    private final NormColumnInfo indexColumn;

    private boolean isIgnoredField( Field f ) {
        if( f.isSynthetic() )
            return true;
        String name = f.getName();
        if( name.startsWith("$") || name.equals("serialVersionUID"))
            return true;
        return false;
    }

    private NormTableInfo(Class<? extends NormTable> tableClass) {
        StringBuilder createTableBuilder = new StringBuilder();
        StringBuilder dropTableBuilder = new StringBuilder();
        tableName = NormDb.camelCaseToLCUnderscore(tableClass.getSimpleName());
        createTableBuilder
                .append("CREATE TABLE ")
                .append(tableName)
                .append(" (\n    ");
        dropTableBuilder
                .append("DROP TABLE IF EXISTS ")
                .append(tableName);
        Field fields[] = tableClass.getDeclaredFields();
        int ignoreCount = 0;
        for( Field f: fields ) {
            if( isIgnoredField(f) )
                ignoreCount++;
        }
        columns = new NormColumnInfo[fields.length-ignoreCount];
        columnNames = new String[fields.length-ignoreCount];
        int indexCount = 0;
        String[] createIndex = null;
        String[] dropIndex = null;
        NormColumnInfo indexColumn = null;
        for (int colidx = -1, fldidx = 0; fldidx < fields.length; fldidx++) {
            Field f = fields[fldidx];
            if( isIgnoredField(f) )
                continue; // Skip generated fields
            colidx++;
            NormColumnInfo columnInfo = new NormColumnInfo(f);
            columns[colidx] = columnInfo;
            columnNames[colidx] = columnInfo.columnName;

            if( columnInfo.index ) {
                if( createIndex==null ) {
                    createIndex = new String[fields.length];
                    dropIndex = new String[fields.length];
                }
                createIndex[indexCount] =
                        "CREATE INDEX idx_" + columnInfo.columnName + " ON " + tableName + "( " + columnInfo.columnName + ")";
                dropIndex[indexCount] =
                        "DROP INDEX IF EXISTS idx_" + columnInfo.columnName;
                indexCount++;
            }

            if( columnInfo.rowId )
                indexColumn = columnInfo;
            if (colidx > 0)
                createTableBuilder.append(",\n    ");
            createTableBuilder.append(columnInfo.columnName).append(' ');
            switch (columnInfo.columnType) {
                case INT:
                case LONG:
                case BOOL:
                    if( columnInfo.rowId ) {
                        createTableBuilder.append("INTEGER PRIMARY KEY");
                    } else {
                        createTableBuilder.append("INTEGER");
                    }
                    break;
                case DOUBLE:
                case FLOAT:
                    createTableBuilder.append("REAL");
                    break;
                case ENUM:
                case TEXT:
                case JSON:
                    createTableBuilder.append("TEXT");
                    break;
                case BLOB:
                    createTableBuilder.append("BLOB");
                    break;
            }
            if (columnInfo.unique) {
                createTableBuilder.append(" UNIQUE");
            }
            if (columnInfo.notNull) {
                createTableBuilder.append(" NOT NULL");
            }
        }
        this.indexColumn = indexColumn;
        createTableBuilder.append(')');
        createTableSQL = new String[indexCount+1];
        createTableSQL[0] = createTableBuilder.toString();
        if( indexCount>0 ) {
            System.arraycopy(createIndex,0,createTableSQL,1,indexCount);
        }
        dropTableSQL = new String[indexCount+1];
        dropTableSQL[indexCount] = dropTableBuilder.toString();
        if( indexCount>0 ) {
            System.arraycopy(dropIndex,0,dropTableSQL,0,indexCount);
        }
    }

    private static Map<Class<? extends NormTable>, NormTableInfo> tableInfoMap = new HashMap<>();

    public static NormTableInfo getTableInfo(Class<? extends NormTable> tableClass) {
        NormTableInfo tableInfo = tableInfoMap.get(tableClass);
        if (tableInfo == null) {
            tableInfo = new NormTableInfo(tableClass);
            tableInfoMap.put(tableClass, tableInfo);
        }
        return tableInfo;
    }

    public String[] getCreateTableSQL() {
        return createTableSQL;
    }

    public String[] getDropTableSQL() {
        return dropTableSQL;
    }

    public String getTableName() {
        return tableName;
    }

    public String[] getColumnNames() {
        return columnNames;
    }

    public NormColumnInfo[] getColumns() {
        return columns;
    }

    public NormColumnInfo getColumnFromName(String columnName) {
        for( NormColumnInfo c: columns ) {
            if( c.columnName.equals(columnName) )
                return c;
        }
        return null;
    }

    public NormColumnInfo getIndexColumn() {
        return indexColumn;
    }


    public String[] getUpgradeTableAddColumnSQL(int oldVersion , int newVersion ){

        ArrayList<Integer> upgradeColumnIndex = new ArrayList<>();

        for(int i = 0 ; i< columns.length ; i++ ) {
            if( columns[i].addColumnVersion > oldVersion ) {
                upgradeColumnIndex.add(i);
            }
        }

        if( upgradeColumnIndex.size() ==0 ){
            return new String[0];
        }

        String[] upgradeSqls = new String[upgradeColumnIndex.size()];
        int count = 0;
        for(int index : upgradeColumnIndex){
            NormColumnInfo info = columns[index];
            StringBuilder upgradeTableBuilder = new StringBuilder();
            upgradeTableBuilder
                    .append("ALTER TABLE ")
                    .append(tableName)
                    .append(" ADD COLUMN ");

            upgradeTableBuilder.append(info.columnName).append(" ");

            switch (info.columnType) {
                case INT:
                case LONG:
                case BOOL:
                    if (info.rowId) {
                        upgradeTableBuilder.append("INTEGER PRIMARY KEY");
                    } else {
                        upgradeTableBuilder.append("INTEGER");
                    }
                    break;
                case DOUBLE:
                case FLOAT:
                    upgradeTableBuilder.append("REAL");
                    break;
                case ENUM:
                case TEXT:
                case JSON:
                    upgradeTableBuilder.append("TEXT");
                    break;
                case BLOB:
                    upgradeTableBuilder.append("BLOB");
                    break;
            }
            upgradeSqls[count] = upgradeTableBuilder.toString();
            upgradeTableBuilder.reverse();
            count++;
        }
        return upgradeSqls;
    }
}
