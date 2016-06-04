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
		GridBagLayout gbl_contentPane = new GridBagLayout();
		gbl_contentPane.columnWidths = new int[]{128, 0, 0};
		gbl_contentPane.rowHeights = new int[]{48, 23, 23, 0};
		gbl_contentPane.columnWeights = new double[]{1.0, 0.0, Double.MIN_VALUE};
		gbl_contentPane.rowWeights = new double[]{0.0, 0.0, 0.0, Double.MIN_VALUE};
		contentPane.setLayout(gbl_contentPane);
		
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
		GridBagConstraints gbc_drawBounds = new GridBagConstraints();
		gbc_drawBounds.anchor = GridBagConstraints.NORTH;
		gbc_drawBounds.fill = GridBagConstraints.HORIZONTAL;
		gbc_drawBounds.insets = new Insets(0, 0, 5, 5);
		gbc_drawBounds.gridx = 0;
		gbc_drawBounds.gridy = 0;
		contentPane.add(drawBounds, gbc_drawBounds);


		GridBagConstraints gbc_drawMask = new GridBagConstraints();
		gbc_drawMask.insets = new Insets(0, 0, 5, 5);
		gbc_drawMask.anchor = GridBagConstraints.NORTH;
		gbc_drawMask.fill = GridBagConstraints.HORIZONTAL;
		gbc_drawMask.gridx = 0;
		gbc_drawMask.gridy = 1;
		contentPane.add(drawMask, gbc_drawMask);
		
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
		GridBagConstraints gbc_comboBox = new GridBagConstraints();
		gbc_comboBox.insets = new Insets(0, 0, 0, 5);
		gbc_comboBox.fill = GridBagConstraints.HORIZONTAL;
		gbc_comboBox.gridx = 0;
		gbc_comboBox.gridy = 2;
		contentPane.add(appSelect, gbc_comboBox);
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
