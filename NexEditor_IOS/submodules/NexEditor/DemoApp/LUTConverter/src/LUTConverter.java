import java.awt.Frame;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.ArrayList;
//import java.util.regex.Pattern;

import javax.imageio.ImageIO;

public class LUTConverter extends Frame{

	private static final long serialVersionUID = 1L;
	int result_width = 64;
	int result_Height = 4096;
	int block_length = 64;
	int block_count = 8;
	int block_totalcount = 64 -1;
	
	public LUTConverter(String[] args) {
		try {
			ConvertFiles(args[0], args[1] );
		}catch(Exception e) {
			e.printStackTrace();
		}
	}

	public static ArrayList<File> listFilesForFolder(final File folder,final boolean recursivity,final String patternFileFilter) {

		boolean filteredFile = false;

		final ArrayList<File> output = new ArrayList<File> ();
	    
	    if(!folder.isDirectory()) {
	        if (patternFileFilter.length() == 0) {
                filteredFile = true;
            }
            else {
                filteredFile = folder.getName().matches(patternFileFilter);
            }
            if (filteredFile) {
                output.add(folder);
            }    	
	    }
	    else {
		    for (final File fileEntry : folder.listFiles()) {
		        if (fileEntry.isDirectory()) {
		            if (recursivity) {
		                output.addAll(listFilesForFolder(fileEntry, recursivity, patternFileFilter));
		            }
		        }
		        else {
		            if (patternFileFilter.length() == 0) {
		                filteredFile = true;
		            }
		            else {
		                filteredFile = fileEntry.getName().matches(patternFileFilter);
		            }
		            if (filteredFile) {
		                output.add(fileEntry);
		            }
		        }
		    }
	    }
	    return output;
	}
	
	private void ConvertFiles(String dest_path, String src_path ) throws Exception {	
		
		if(!src_path.isEmpty()) {
			File directory = new File(src_path);
			if(directory.exists()){
				System.out.println("Already exist");
			} 
			else {
				if(directory.mkdir()) {			
					System.out.println(directory.getAbsolutePath());
				} 
				else {
					System.out.println("Directory create failure!");
				}
			}
			String check = src_path.substring(src_path.length()-1, src_path.length());
			if(!check.equals("\\"))
				src_path = src_path + "\\"; 
		}
	
		if(dest_path.isEmpty()){
			dest_path = ".";
		}
		
		File dest_file = new File(dest_path);
		ArrayList<File> entry_list = listFilesForFolder( dest_file, true, ".*png");
		
		for (File entry : entry_list)
		{
			BufferedImage dest_image = ImageIO.read(entry);
			if(dest_image.getWidth() != block_length * block_count || dest_image.getHeight() != block_length * block_count)
	        {
				System.out.println(String.format("miss matching dest file : %s", entry.getName()));
				continue;
	        }
			
			BufferedImage result_image = convertImage(dest_image);			
		    File resultFile = new File(src_path + "converted_"+ entry.getName());
	        ImageIO.write(result_image, "png", resultFile);			
		}
		System.out.println("convert exist");
	}
	
	private BufferedImage convertImage(BufferedImage dest_image) throws Exception{
		BufferedImage result_image = new BufferedImage(result_width, result_Height, BufferedImage.TYPE_INT_ARGB );
        int block_index = 0;
        for(int i = 0; i < block_count; i++){
        	for(int j = 0; j < block_count; j++) {
        		block_index = block_totalcount - (( i * block_count ) + j);
        		for(int x = 0; x < block_length; x++){
        			for(int y = 0; y < block_length; y++){
                		int p = dest_image.getRGB((block_length - 1) - y + (j * block_length), x + (i * block_length));
                		result_image.setRGB(x, y + (block_index * block_length), p);
                	}
                }        		
        	}
        }
        return result_image;
	}
	
	public static void main(String[] args) {
		new LUTConverter(args);       
	}
}
