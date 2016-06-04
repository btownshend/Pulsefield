import java.awt.BorderLayout;
import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import com.jogamp.opengl.awt.GLCanvas;

import processing.core.PApplet;

import javax.swing.JButton;
import javax.swing.AbstractAction;
import java.awt.event.ActionEvent;
import javax.swing.Action;
import java.awt.event.ActionListener;
import java.awt.FlowLayout;
import java.awt.Canvas;
import javax.swing.JCheckBox;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import javax.swing.JComboBox;
import javax.swing.DefaultComboBoxModel;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

@SuppressWarnings("serial")
public class GUI extends JFrame {
	public static GUI theGUI;
	private JPanel contentPane;
	private JCheckBox drawMasks, drawBounds;
	@SuppressWarnings("rawtypes")
	private JComboBox appSelect;
	private boolean initialized=false;
	private JCheckBox useMasks;
	
	public static void start() {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					theGUI = new GUI();
					theGUI.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public GUI() {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 450, 300);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		
		drawMasks = new JCheckBox("Draw Mask");

		drawMasks.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.drawMasks=drawMasks.isSelected();
			}
		});
		
		drawBounds = new JCheckBox("Draw Bounds");
		drawBounds.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.drawBounds=drawBounds.isSelected();
				PApplet.println("drawBounds="+drawBounds.toString());
			}
		});
		contentPane.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		contentPane.add(drawBounds);
		contentPane.add(drawMasks);
		
		appSelect = new JComboBox();
		appSelect.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.setapp(appSelect.getSelectedIndex());
			}
		});
		
		useMasks = new JCheckBox("Use Mask");
		useMasks.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.useMasks=useMasks.isSelected();
			}
		});
		contentPane.add(useMasks);
		appSelect.setModel(new DefaultComboBoxModel(Tracker.visnames));
		contentPane.add(appSelect);
		initialized=true;
		update();
	}
	
	void update() {
		PApplet.println("update called");
		if (!initialized) {
			PApplet.println("update when not initialized");
			return;
		}
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					PApplet.println("update run");
					drawMasks.setSelected(Tracker.theTracker.drawMasks);
					drawBounds.setSelected(Tracker.theTracker.drawBounds);
					useMasks.setSelected(Tracker.theTracker.useMasks);
					appSelect.setSelectedIndex(Tracker.theTracker.currentvis);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
}
