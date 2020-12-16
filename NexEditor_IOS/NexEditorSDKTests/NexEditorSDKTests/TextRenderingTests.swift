/******************************************************************************
 * File Name   : TextRenderingTests.swift
 * Description :
 *******************************************************************************
 
 NexStreaming Corp. Confidential & Proprietary
 Copyright (C) 2017 NexStreaming Corp. All rights are reserved.
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
import XCTest
import Foundation
@testable import NexEditorFramework


class TextRenderingTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testNESI251_CustomFontsAscenderHigherThanCapHeight() {
        
        let fontFileNames = [ "bevan", "creepster-regular", "goudystm-italic", "greatvibes-regular", "junctionregular", "knewave", "lato-bold",
        "leaguegothic", "leaguescriptthin-leaguescript", "lindenhill", "orbitron-medium", "redressed", "releway-thin", "sniglet"];
        
        for fileName in fontFileNames {
            if let fontName = FontTools.fontName(withBundleFontName: fileName) {
                let baseFont = UIFont(name: fontName, size: UIFont.systemFontSize)!
                
                XCTAssert((baseFont.ascender - baseFont.capHeight) > -(baseFont.capHeight * 0.07),
                          String(format:"Ascender(%2.1f) must be greater or equal to 7 percent of capHeight(%2.1f) for font:%@",
                          baseFont.ascender, baseFont.capHeight, fontName));
            } else {
                XCTFail("Font name not resolved for " + fileName)
            }
        }
    }
    
    func testNESI251_FontEmojiJunction() {
        /**
         * Test requires visual observation on resulting 'image'. Successful result shows top of Emoji characters are not clipped out.
         * How to test:
         * 1. Set break point at the print(...) line
         * 2. Run test
         * 3. If it stops at the break point, select 'image' symble from the debugger watch and click the 'eye' icon to preview
         * 4. Test failed if top of emojis are clipped out
         */
        let fontFileName = "junctionregular";
        let text = "😘😙😀😌😒😭ABCD😘😙😀😌😒😭abcd😘😙😀😌😒😭가나다라😘😙😀😌😒😭";
        
        let (image, fontSize, textBoundsSize) = inspectRendering(text, with: fontFileName)
        
        print("fontSize: \(fontSize) textBoundsSize: \(textBoundsSize) image: \(image)")
        
    }
    
    func testNESI251_FontEmojiLeagueScriptThin() {
        /**
         * Test requires visual observation on resulting 'image'. Successful result shows top of Emoji characters are not clipped out.
         * How to test:
         * 1. Set break point at the print(...) line
         * 2. Run test
         * 3. If it stops at the break point, select 'image' symble from the debugger watch and click the 'eye' icon to preview
         * 4. Test failed if top of emojis are clipped out
         */
        let fontFileName = "leaguescriptthin-leaguescript";
        let text = "😘😙😀😌😒😭ABCD😘😙😀😌😒😭abcd😘😙😀😌😒😭가나다라😘😙😀😌😒😭";
        
        let (image, fontSize, textBoundsSize) = inspectRendering(text, with: fontFileName)
        
        print("fontSize: \(fontSize) textBoundsSize: \(textBoundsSize) image: \(image)")
        
    }
    
    func inspectRendering(_ text: String, with fontFileName: String) -> (UIImage, CGFloat, CGSize) {
        var size_ = 90.0;
        let maxLines_ = 0;
        let imageSize = CGSize(width: 300, height: 30)
        
        let scale: Double = Double(UIScreen.main.scale);
        size_ =  size_ / scale;
        
        
        var textAttributes: [String: Any] = [NSParagraphStyleAttributeName: defaultParagraphStyle()];
        
        let contextSize = CGSize(width: imageSize.width, height: imageSize.height )
        UIGraphicsBeginImageContextWithOptions(contextSize, false , CGFloat(scale))
        
        let context = UIGraphicsGetCurrentContext()!
        context.setFillColor(UIColor.clear.cgColor)
        context.fill(CGRect(x: 0, y: 0, width: contextSize.width, height: contextSize.height))
        
        var textRegionSize = CGSize(width: 0, height: 0)
        let fontSize = FontTools.adjustFontSize(withFontName: fontFileName,
                                       textRegionSize: imageSize,
                                       textSize: CGFloat(size_),
                                       maxLines: Int32(maxLines_),
                                       text: text,
                                       destTextRegion: &textRegionSize)
        let fontName = FontTools.fontName(withBundleFontName: fontFileName)!
        
        textAttributes[NSFontAttributeName] = UIFont(name: fontName, size: fontSize)
        let t = text as NSString
        
        t.draw(with: CGRect(x: 0, y: 0, width: textRegionSize.width, height: textRegionSize.height),
               options: [.usesFontLeading, .usesLineFragmentOrigin],
               attributes: textAttributes as [String: Any],
               context: nil)
        
        
        let image = UIGraphicsGetImageFromCurrentImageContext()!;
        UIGraphicsEndImageContext();
        
        return (image, fontSize, textRegionSize)
    }
    
    func defaultParagraphStyle() -> NSParagraphStyle {
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.lineBreakMode = .byWordWrapping
        paragraphStyle.lineSpacing = 0.5
        paragraphStyle.lineHeightMultiple = 1.1
        paragraphStyle.alignment = .center
        
        return paragraphStyle.copy() as! NSParagraphStyle
    }
}
